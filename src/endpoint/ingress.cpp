//
// Created by zanna on 05/10/18.
//

#include "ingress.h"

endpoint::Ingress::Ingress(uint16_t ext_port, uint16_t int_port) :
        Endpoint(ext_port, int_port){}

void endpoint::Ingress::manage_entering_tcp_packets(void * mngmnt_args) {
    LOG(ldebug, "manage tcp packet");

    auto args = (server::tcp::tcp_pkt_mngmnt_args*)mngmnt_args;
    int new_socket_fd = args->new_socket_fd;

    ssize_t read_size;
    unsigned char pkt[BUFFER_SIZE];
    char* sfcid, * prev_sfcid = const_cast<char*>("");
    bool first_pkt = true;
    bool db_error = false;
    char* next_ip;
    uint16_t next_port;
    unsigned long ttl;


    do {
        printf("Hello message sent\n");
        read_size = read(new_socket_fd, pkt, BUFFER_SIZE);
        for (int i = 0; i < read_size; i++)
            printf("%x", *(pkt + i));
        printf("\n");
        if (read_size < 0) {
            perror("error receiving data");
            exit(EXIT_FAILURE);
        } else if (read_size > 0) {
            LOG(ldebug, "6");
            sfcid = Endpoint::classifier_.classify_pkt((unsigned char*) pkt, read_size);
            if (first_pkt || (strcmp(sfcid, prev_sfcid) != 0)) {
                auto headers =
                        utils::PacketUtils::retrieve_ip_tcp_header(
                                (unsigned char *) pkt);
                try {
                    add_entry(ConnectionEntry(
                            INT_TO_IP(headers.first.saddr),
                            INT_TO_IP(headers.first.daddr),
                            htons(headers.second.source),
                            htons(headers.second.dest), sfcid,
                            db::protocol_type::TCP),
                              new_socket_fd,
                              args->client_address,
                              db::endpoint_type::INGRESS_T,
                              db::protocol_type::TCP);

                    std::vector <db::utils::Address> path =
                            roulette_->get_route_list(atoi(sfcid));

                    if (!path.empty()) {
                        // +2 because of ingress & egress
                        ttl = path.size() + 2;
                        next_ip = const_cast<char *>(path[0].get_address().c_str());
                        next_port = path[0].get_port();

                        sfc_header flh =
                                utils::sfc_header::SFCUtilities::create_header(
                                        atoi(sfcid), 0,
                                        INT_TO_IP_C_STR(headers.first.saddr),
                                        headers.second.source,
                                        INT_TO_IP_C_STR(headers.first.daddr),
                                        headers.second.dest, ttl, 0);
                        unsigned char formatted_pkt[read_size + SFC_HDR];
                        unsigned char *p = formatted_pkt;
                        utils::sfc_header::SFCUtilities::prepend_header(pkt,
                                                                        read_size,
                                                                        flh, p);


                        client::udp::ClientUDP().send_and_wait_response(p,
                                                                        read_size +
                                                                        SFC_HDR,
                                                                        next_ip,
                                                                        next_port);

                        prev_sfcid = sfcid;
                        first_pkt = false;
                    } else {
                        LOG(ldebug, "no route available, discarding packages");
                        break;
                    }
                } catch (db::exceptions::ios_base::failure &e) {
                    perror(e.what());
                    db_error = true;
                }
            }
        }

        if (read_size < BUFFER_SIZE) {
            break;
        }

    } while(1);

    if(read_size == 0) {
        puts("Client disconnected");
        fflush(stdout);
    }
    else if(read_size == -1) {
        perror("recv failed");
    }

    free(args);

    if (db_error)
        close(new_socket_fd);
}

void endpoint::Ingress::manage_entering_udp_packets(void * mngmnt_args) {
    LOG(ldebug, "manage udp packet");

    auto args = (server::udp::udp_pkt_mngmnt_args *)mngmnt_args;

    char* sfcid = Endpoint::classifier_.classify_pkt((unsigned char*) args->pkt,
                                           args->pkt_len);
    auto headers =
            utils::PacketUtils::retrieve_ip_udp_header(
                    (unsigned char*)args->pkt);

    add_entry(ConnectionEntry(
                    INT_TO_IP(headers.first.saddr),
                    INT_TO_IP(headers.first.daddr),
                    htons(headers.second.source),
                    htons(headers.second.dest),
                    sfcid, db::protocol_type::UDP),
               args->socket_fd,
               args->client_address,
               db::endpoint_type::INGRESS_T,
               db::protocol_type::UDP);

    std::vector<db::utils::Address> path =
            roulette_->get_route_list(atoi(sfcid));

    if (!path.empty()) {
        // +2 because of ingress & egress
        unsigned long ttl = path.size() + 2;
        char* next_ip = const_cast<char*>(path[0].get_address().c_str());
        uint16_t next_port = path[0].get_port();

        sfc_header flh =
                utils::sfc_header::SFCUtilities::create_header(
                        atoi(sfcid), 0,
                        INT_TO_IP_C_STR(headers.first.saddr),
                        headers.second.source,
                        INT_TO_IP_C_STR(headers.first.daddr),
                        headers.second.dest, ttl, 0);
        unsigned char formatted_pkt[args->pkt_len + SFC_HDR];
        unsigned char* p = formatted_pkt;
        utils::sfc_header::SFCUtilities::prepend_header((unsigned char*)args->pkt,
                                                        args->pkt_len,
                                                        flh, p);

        client::udp::ClientUDP().send_and_wait_response(formatted_pkt,
                                                        args->pkt_len + SFC_HDR,
                                                        const_cast<char*>(next_ip),
                                                        next_port);
    } else {
        LOG(ldebug, "no route available, discarding packages");
    }
    delete(args->pkt);
    free(args);
}

void endpoint::Ingress::manage_pkt_from_chain(void * mngmnt_args) {
    LOG(ldebug, "manage packet from chain");
    auto args = (server::udp::udp_pkt_mngmnt_args *)mngmnt_args;

    sendto(args->socket_fd,
           ACK,
           ACK_SIZE,
           0,
           reinterpret_cast<struct sockaddr*>(&args->client_address),
           sizeof(args->client_address));



    struct sfc_header header =
            utils::sfc_header::SFCUtilities::retrieve_header((uint8_t*)args->pkt);

    struct iphdr ip_hdr =
            utils::PacketUtils::retrieve_ip_header(
                    (unsigned char*)(args->pkt + SFC_HDR));
    // TODO think something more extensible to support more protocols
    // '6' is the code for TCP '17' for UDP
    db::protocol_type conn_type = (ip_hdr.protocol == 6?
                                   db::protocol_type::TCP :
                                   db::protocol_type::UDP);

    LOG(ldebug, "from chain type: " + std::to_string(conn_type));

    ConnectionEntry ce(
            INT_TO_IP(header.source_address),
            INT_TO_IP(header.destination_address),
            htons(header.source_port),
            htons(header.destination_port),
            std::to_string(header.p_id), conn_type);

    std::pair<endpoint::socket_fd, sockaddr_in> sock = retrieve_connection_2(ce);

    LOG(ldebug, "retrieved: " + sock.first);

    ssize_t resp_c;
    // to calculate payload pointer (missing tcp/udp header size)
    unsigned int pkt_calc = SFC_HDR;
    if (conn_type == db::protocol_type::TCP) {

        printf("Header size: %d\n", IP_TCP_H_LEN(args->pkt + SFC_HDR));
        pkt_calc += IP_TCP_H_LEN(args->pkt + SFC_HDR);
        send(sock.first , args->pkt + pkt_calc, args->pkt_len - pkt_calc, 0);
    } else {
        printf("Header size: %d\n", IP_UDP_H_LEN(args->pkt + SFC_HDR));
        pkt_calc += IP_UDP_H_LEN(args->pkt + SFC_HDR);
        socklen_t s = sizeof(sock.second);
        resp_c = sendto(sock.first, args->pkt + pkt_calc,
                        args->pkt_len - pkt_calc, 0,
                        (struct sockaddr *)&sock.second, s);
        if (resp_c < 0) {
            perror("1. sendto()");
            exit(EXIT_FAILURE);
        }
    };

    delete(args->pkt);
    free(args);
}

void endpoint::Ingress::start() {
    uint16_t int_port = get_internal_port();
    uint16_t ext_port = get_external_port();

    std::function<void (void*)> vnf_callback = [this](void* args) {
        this->manage_pkt_from_chain(args);
    };
    std::thread udp_internal_thread([&int_port, &vnf_callback]{
        LOG(ldebug, "starting udp server for internal pkts on port "
                    + std::to_string(int_port));
        auto server = new server::udp::ServerUDP(int_port);
        server->set_pkt_manager(vnf_callback);
        server->run();
    });

    std::function<void (void*)> udp_callback = [this](void* args) {
        this->manage_entering_udp_packets(args);
    };
    std::thread udp_external_thread([&ext_port, &udp_callback]{
        LOG(ldebug, "starting udp server for external pkts on port "
                    + std::to_string(ext_port));
        auto server = new server::udp::ServerUDP(ext_port);
        server->set_pkt_manager(udp_callback);
        server->run();
    });

    std::function<void (void*)> tcp_callback = [this](void* args) {
        this->manage_entering_tcp_packets(args);
    };
    std::thread tcp_thread([&ext_port, &tcp_callback]{
        LOG(ldebug, "starting tcp server for external pkts on port "
                    + std::to_string(ext_port));
        auto server = new server::tcp::ServerTCP(ext_port);
        server->set_pkt_manager(tcp_callback);
        server->run();
    });

    tcp_thread.join();
    udp_external_thread.join();
    udp_internal_thread.join();
}

