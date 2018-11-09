#include "ingress.h"

endpoint::Ingress::Ingress(uint16_t ext_port, uint16_t int_port) :
        Endpoint(ext_port, int_port) {}

void endpoint::Ingress::manage_entering_tcp_packets(void *mngmnt_args) {
    LOG(ldebug, "manage tcp packet");

    auto args = (server::tcp::tcp_pkt_mngmnt_args *) mngmnt_args;
    int new_socket_fd = args->new_socket_fd;
    ssize_t read_size;
    unsigned char pkt[BUFFER_SIZE];
    char *sfcid = const_cast<char *>(""), *prev_sfcid = const_cast<char *>("");
    bool db_error = false;
    uint16_t next_port;
    unsigned long ttl;

    /*for (int i = 0; i < args->pkt_size; i++)
        printf("%c", (args->pkt)[i]);
    printf("\n");*/

    int s = client::udp::ClientUDP().send_only((unsigned char*)args->pkt,
                                                    args->pkt_size,
                                                    "127.0.0.1",
                                                    9090);
    close(s);
    /*std::pair<iphdr, tcphdr> headers;
    read_size = recv(new_socket_fd, pkt, BUFFER_SIZE, 0);
    if (read_size < 0) {
        perror("error receiving data");
        exit(EXIT_FAILURE);
    } else if (read_size > 0) {
        LOG(ltrace, "Read size > 0, data incoming");
        sfcid = "0";//Endpoint::classifier_.classify_pkt((unsigned char *) pkt,
                    //                               static_cast<size_t>(read_size));

        headers =
                utils::PacketUtils::retrieve_ip_tcp_header(
                        (unsigned char *) pkt);


        if (args->first_run) {
            LOG(ltrace, "First run, adding entry");
            args->ce = new ConnectionEntry(
                        INT_TO_IP(headers.first.saddr),
                        INT_TO_IP(headers.first.daddr),
                        htons(headers.second.source),
                        htons(headers.second.dest), sfcid,
                        db::protocol_type::TCP);
            add_entry(*args->ce,
                      new_socket_fd,
                      args->client_address,
                      db::endpoint_type::INGRESS_T,
                      db::protocol_type::TCP);
        }
        try {

            std::vector<db::utils::Address> path =
                    roulette_->get_route_list(
                            static_cast<uint32_t>(std::stoi(sfcid)));

            if (!path.empty()) {
                // +2 because of ingress & egress
                //ttl = path.size() + 2;
                next_port = 9090;//static_cast<uint16_t>(path[0].get_port());

                sfc_header flh =
                        utils::sfc_header::SFCUtilities::create_header(
                                static_cast<uint32_t>(std::stoi(sfcid)),
                                0,
                                INT_TO_IP_C_STR(headers.first.saddr),
                                headers.second.source,
                                INT_TO_IP_C_STR(headers.first.daddr),
                                headers.second.dest,
                                static_cast<uint16_t>(ttl), 0);
                std::vector<unsigned char> formatted_pkt(
                        static_cast<unsigned long>(read_size +
                                                   SFC_HDR));
                unsigned char *p = &formatted_pkt[0];
                utils::sfc_header::SFCUtilities::prepend_header(pkt,
                                                                static_cast<size_t>(read_size),
                                                                flh, p);
                client::udp::ClientUDP().send_and_wait_response(p,
                                                                static_cast<size_t>(
                                                                        read_size +
                                                                        SFC_HDR),
                                                                path[0].get_address().c_str(),
                                                                next_port);

                client::udp::ClientUDP().send_and_wait_response((unsigned char*)args->pkt,
                                                                args->pkt_size,
                                                                "192.168.1.6",
                                                                9090);
                prev_sfcid = sfcid;
            } else {
                LOG(ldebug, "no route available, discarding packages");
            }
        } catch (db::exceptions::ios_base::failure &e) {
            LOG(lwarn, "Encountered an ios_base failure");
            perror(e.what());
            db_error = true;
        }
    }*/

    /*if (read_size == 0) {
        puts("Client disconnected");
        fflush(stdout);
        if (args->ce) {
            delete_entry(*args->ce);
            delete args->ce;
        } else {
            LOG(ldebug, "No connection entry set, nothing to delete");
        }
        close(new_socket_fd);
        free(args);
    } else if (read_size == -1) {
        perror("recv failed");
    }*/

    //free(args);

    /*if (db_error) {
        LOG(ltrace, "Close ln 106 ingress.cpp");
        close(new_socket_fd);
    }*/
}

void endpoint::Ingress::manage_entering_udp_packets(void *mngmnt_args) {
    LOG(ldebug, "manage udp packet");

    auto args = (server::udp::udp_pkt_mngmnt_args *) mngmnt_args;

    char *sfcid = Endpoint::classifier_.classify_pkt(
            (unsigned char *) args->pkt,
            static_cast<size_t>(args->pkt_len));
    auto headers =
            utils::PacketUtils::retrieve_ip_udp_header(
                    (unsigned char *) args->pkt);

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
            roulette_->get_route_list(static_cast<uint32_t>(std::stoi(sfcid)));

    if (!path.empty()) {
        // +2 because of ingress & egress
        unsigned long ttl = path.size() + 2;
        auto next_port = static_cast<uint16_t>(path[0].get_port());

        sfc_header flh =
                utils::sfc_header::SFCUtilities::create_header(
                        static_cast<uint32_t>(std::stoi(sfcid)), 0,
                        INT_TO_IP_C_STR(headers.first.saddr),
                        headers.second.source,
                        INT_TO_IP_C_STR(headers.first.daddr),
                        headers.second.dest, static_cast<uint16_t>(ttl), 0);
        std::vector<unsigned char> formatted_pkt(
                static_cast<unsigned long>(args->pkt_len + SFC_HDR));
        unsigned char *p = &formatted_pkt[0];
        utils::sfc_header::SFCUtilities::prepend_header(
                (unsigned char *) args->pkt,
                static_cast<size_t>(args->pkt_len),
                flh, p);

        client::udp::ClientUDP().send_and_wait_response(p,
                                                        static_cast<size_t>(
                                                                args->pkt_len +
                                                                SFC_HDR),
                                                        path[0].get_address().c_str(),
                                                        next_port);
    } else {
        LOG(ldebug, "no route available, discarding packages");
    }
    delete (args->pkt);
    //free(args);
}

void endpoint::Ingress::manage_pkt_from_chain(void *mngmnt_args) {
    LOG(ldebug, "manage packet from chain");
    auto args = (server::udp::udp_pkt_mngmnt_args *) mngmnt_args;

    sendto(args->socket_fd,
           ACK,
           ACK_SIZE,
           0,
           reinterpret_cast<struct sockaddr *>(&args->client_address),
           sizeof(args->client_address));


    struct sfc_header header =
            utils::sfc_header::SFCUtilities::retrieve_header(
                    (uint8_t *) args->pkt);

    struct iphdr ip_hdr =
            utils::PacketUtils::retrieve_ip_header(
                    (unsigned char *) (args->pkt + SFC_HDR));
    // TODO think something more extensible to support more protocols
    // '6' is the code for TCP '17' for UDP
    db::protocol_type conn_type = (ip_hdr.protocol == 6 ?
                                   db::protocol_type::TCP :
                                   db::protocol_type::UDP);

    LOG(ldebug, "from chain type: " + std::to_string(conn_type));

    ConnectionEntry ce(
            INT_TO_IP(header.source_address),
            INT_TO_IP(header.destination_address),
            htons(header.source_port),
            htons(header.destination_port),
            std::to_string(header.p_id), conn_type);

    std::pair<endpoint::socket_fd, sockaddr_in> sock = retrieve_connection_2(
            ce);

    LOG(ldebug, "retrieved: " + std::to_string(sock.first));

    ssize_t resp_c;
    // to calculate payload pointer (missing tcp/udp header size)
    unsigned int pkt_calc = SFC_HDR;
    if (conn_type == db::protocol_type::TCP) {
        pkt_calc += IP_TCP_H_LEN(args->pkt + SFC_HDR);
        resp_c = send(sock.first, args->pkt + pkt_calc,
                      static_cast<size_t>(args->pkt_len - pkt_calc), 0);
        if (resp_c < 0) {
            perror("send()");
            exit(EXIT_FAILURE);
        }
        //shutdown(sock.first, 2);
        //LOG(ltrace, "Close ln 221 ingress.cpp");
        //close(sock.first);
        LOG(ltrace, "Not closing ln 221 ingress.cpp");
    } else {
        pkt_calc += IP_UDP_H_LEN(args->pkt + SFC_HDR);
        socklen_t s = sizeof(sock.second);
        resp_c = sendto(sock.first, args->pkt + pkt_calc,
                        static_cast<size_t>(args->pkt_len - pkt_calc), 0,
                        (struct sockaddr *) &sock.second, s);
        if (resp_c < 0) {
            perror("1. sendto()");
            exit(EXIT_FAILURE);
        }
        delete_entry(ConnectionEntry(
                INT_TO_IP(header.source_address),
                INT_TO_IP(header.destination_address),
                htons(header.source_port),
                htons(header.destination_port), std::to_string(header.p_id),
                db::protocol_type::UDP));
    };

    delete (args->pkt);
    //free(args);
}

void endpoint::Ingress::start() {
    uint16_t int_port = get_internal_port();
    uint16_t ext_port = get_external_port();

    std::function<void(void *)> vnf_callback = [this](void *args) {
        this->manage_pkt_from_chain(args);
    };
    std::thread udp_internal_thread([&int_port, &vnf_callback] {
        LOG(ldebug, "starting udp server for internal pkts on port "
                    + std::to_string(int_port));
        auto server = new server::udp::ServerUDP(int_port);
        server->set_pkt_manager(vnf_callback);
        server->run();
    });

    std::function<void(void *)> udp_callback = [this](void *args) {
        this->manage_entering_udp_packets(args);
    };
    std::thread udp_external_thread([&ext_port, &udp_callback] {
        LOG(ldebug, "starting udp server for external pkts on port "
                    + std::to_string(ext_port));
        auto server = new server::udp::ServerUDP(ext_port);
        server->set_pkt_manager(udp_callback);
        server->run();
    });

    std::function<void(void *)> tcp_callback = [this](void *args) {
        this->manage_entering_tcp_packets(args);
    };
    std::thread tcp_thread([&ext_port, &tcp_callback] {
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

