//
// Created by zanna on 05/10/18.
//

#include "ingress.h"

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
    unsigned char* formatted_pkt;
    unsigned long ttl;
    while((read_size = recv(new_socket_fd, pkt, BUFFER_SIZE, 0)) > 0 && !db_error) {
        sfcid = Endpoint::classifier_.classify_pkt((unsigned char*) pkt, read_size);

        if (first_pkt || (strcmp(sfcid, prev_sfcid) != 0)) {
            auto headers =
                    utils::PacketUtils::retrieve_ip_tcp_header((unsigned char*)pkt);
            try {
                add_entry(ConnectionEntry(
                                utils::PacketUtils::int_to_ip(headers.first.saddr),
                                utils::PacketUtils::int_to_ip(headers.first.daddr),
                                headers.second.source, headers.second.dest, sfcid),
                          new_socket_fd,
                          db::endpoint_type::INGRESS_T,
                          db::protocol_type::TCP);

                std::vector<db::utils::Address> path =
                        roulette_->get_route_list(atoi(sfcid));
                // +2 because of ingress & egress
                ttl = path.size() + 2;
                next_ip = const_cast<char*>(path[0].get_URL().c_str());
                next_port = path[0].get_port();

                sfc_header flh =
                        utils::sfc_header::SFCUtilities::create_header(
                                atoi(sfcid), 0,const_cast<char*>(
                                        utils::PacketUtils::int_to_ip(
                                                headers.first.saddr).c_str()),
                                headers.second.source,const_cast<char*>(
                                        utils::PacketUtils::int_to_ip(
                                                headers.first.daddr).c_str()),
                                headers.second.dest, ttl, 0);

                utils::sfc_header::SFCUtilities::prepend_header(pkt,read_size,
                                                                flh, formatted_pkt);
                client::udp::ClientUDP().send_and_wait_response(formatted_pkt,
                                                                read_size + SFCHDR_LEN,
                                                                next_ip, next_port);

                prev_sfcid = sfcid;
                first_pkt = false;
            } catch(db::exceptions::ios_base::failure& e) {
                perror(e.what());
                db_error = true;
            }

        }
    }

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

#include <iostream>
void endpoint::Ingress::manage_entering_udp_packets(void * mngmnt_args) {
    LOG(ldebug, "manage udp packet");

    auto args = (server::udp::udp_pkt_mngmnt_args *)mngmnt_args;

    std::string ack = "ACK";

    LOG(ldebug, "manage udp packet sendto" + std::to_string(args->socket_fd));
    sendto(args->socket_fd,
           ack.c_str(),
           ack.length(),
           0,
           reinterpret_cast<struct sockaddr*>(&args->client_address),
           sizeof(args->client_address));

    LOG(ldebug, "ACK sent");

    char* sfcid = Endpoint::classifier_.classify_pkt((unsigned char*) args->pkt,
                                           args->pkt_len);
    LOG(ltrace, "1");
    auto headers =
            utils::PacketUtils::retrieve_ip_udp_header(
                    (unsigned char*)args->pkt);

    LOG(ltrace, "2");

    add_entry(ConnectionEntry(
                    utils::PacketUtils::int_to_ip(headers.first.saddr),
                    utils::PacketUtils::int_to_ip(headers.first.daddr),
                    headers.second.source, headers.second.dest, sfcid),
               args->socket_fd,
               db::endpoint_type::INGRESS_T,
               db::protocol_type::UDP);

    LOG(ltrace, "3");

    std::vector<db::utils::Address> path =
            roulette_->get_route_list(atoi(sfcid));

    for (db::utils::Address a : path) {
        LOG(ltrace, "url " + a.get_URL());
        LOG(ltrace, "port " + std::to_string(a.get_port()));
    }

    LOG(ltrace, "4");

    // +2 because of ingress & egress
    unsigned long ttl = path.size() + 2;
    char next_ip[path[0].get_address().size()];
    strcpy(next_ip, path[0].get_address().c_str());
    uint16_t next_port = path[0].get_port();

    LOG(ltrace, "5");

    LOG(ltrace, next_ip);
    LOG(ltrace, std::to_string(next_port));

    sfc_header flh =
            utils::sfc_header::SFCUtilities::create_header(atoi(sfcid), 0,
                    utils::PacketUtils::int_to_ip(
                            headers.first.saddr).c_str(),
                    headers.second.source,
                    utils::PacketUtils::int_to_ip(
                            headers.first.daddr).c_str(),
                    headers.second.dest, ttl, 0);

    LOG(ltrace, next_ip);
    LOG(ltrace, std::to_string(next_port));

    unsigned char* formatted_pkt = new unsigned char[1];
    utils::sfc_header::SFCUtilities::prepend_header((unsigned char*)args->pkt,
                                                    args->pkt_len,
                                                    flh, formatted_pkt);



    LOG(ltrace, "6");
    LOG(ltrace, next_ip);
    LOG(ltrace, std::to_string(next_port));
    client::udp::ClientUDP().send_and_wait_response(formatted_pkt,
                                                    args->pkt_len + SFCHDR_LEN,
                                                    const_cast<char*>(next_ip), next_port);
    std::cout << formatted_pkt << std::endl;

    LOG(ltrace, "7");
    delete(args->pkt);
    free(args);
}

void endpoint::Ingress::manage_pkt_from_chain(void * mngmnt_args) {
    LOG(ldebug, "manage packet from chain");

    auto args = (server::udp::udp_pkt_mngmnt_args *)mngmnt_args;

    std::string ack = "ACK";
    LOG(ldebug, "manage packet from chain sendto" + std::to_string(args->socket_fd));
    sendto(args->socket_fd,
           ack.c_str(),
           ack.length(),
           0,
           reinterpret_cast<struct sockaddr*>(&args->client_address),
           sizeof(args->client_address));

    LOG(ldebug, "ACK sent");

    struct sfc_header header =
            utils::sfc_header::SFCUtilities::retrieve_header((uint8_t*)args->pkt);

    LOG(ldebug, "1");

    unsigned char* payload;
    utils::sfc_header::SFCUtilities::retrieve_payload((uint8_t*)args->pkt,
                                                      args->pkt_len,
                                                      payload);

    LOG(ldebug, "2");

    endpoint::socket_fd sock = retrieve_connection(ConnectionEntry(
            utils::PacketUtils::int_to_ip(header.source_address),
            utils::PacketUtils::int_to_ip(header.destination_address),
            header.source_port, header.destination_port,
            std::to_string(header.p_id)));

    LOG(ldebug, "3");

    auto headers =
            utils::PacketUtils::retrieve_ip_udp_header(
                    (unsigned char*)(args->pkt + SFCHDR_LEN));
    // TODO think something more extensible to support more protocols
    // '6' is the code for TCP '17' for UDP
    LOG(ldebug, "4");
    db::protocol_type conn_type = (headers.first.protocol == 6?
                                   db::protocol_type::TCP :
                                   db::protocol_type::UDP);
    LOG(ldebug, "5");
    unsigned char resp_buff[BUFFER_SIZE];
    ssize_t resp_c;
    if (conn_type == db::protocol_type::TCP) {
        send(sock , args->pkt, args->pkt_len, 0);
        resp_c = read( sock , resp_buff, BUFFER_SIZE);
        if (resp_c < -1) {
            perror("error receiving data");
            exit(EXIT_FAILURE);
        }
    } else {
        LOG(ldebug, "Recognized UDP protocol");
        struct sockaddr_in server;
        server.sin_family      = AF_INET;
        server.sin_port        = header.destination_port;
        server.sin_addr.s_addr = header.destination_address;
        socklen_t s = sizeof(server);
        LOG(ldebug, "sendto inside manage pkt from chain");
        resp_c = sendto(sock, args->pkt, args->pkt_len, 0,
                        (struct sockaddr *)&server, sizeof(server));
        if (resp_c < 0) {
            perror("sendto()");
            exit(EXIT_FAILURE);
        }

        char buffer[BUFFER_SIZE];
        resp_c = recvfrom(sock,
                          (char *)buffer, BUFFER_SIZE, 0,
                          (struct sockaddr *)&server, &s);
    };

    delete(args->pkt);
    free(args);
}

void endpoint::Ingress::start(uint16_t int_port, uint16_t ext_port) {
    std::function<void (void*)> udp_callback = [this](void* args) {
        this->manage_entering_udp_packets(args);
    };
    std::thread udp_internal_thread([&int_port, &udp_callback]{
        auto server = new server::udp::ServerUDP(int_port);
        server->set_pkt_manager(udp_callback);
        server->run();
    });

    std::function<void (void*)> vnf_callback = [this](void* args) {
        this->manage_pkt_from_chain(args);
    };
    std::thread udp_external_thread([&ext_port, &vnf_callback]{
        auto server = new server::udp::ServerUDP(ext_port);
        server->set_pkt_manager(vnf_callback);
        server->run();
    });

    std::function<void (void*)> tcp_callback = [this](void* args) {
        this->manage_entering_tcp_packets(args);
    };
    std::thread tcp_thread([&ext_port, &tcp_callback]{
        auto server = new server::tcp::ServerTCP(ext_port);
        server->set_pkt_manager(tcp_callback);
        server->run();
    });

    tcp_thread.join();
    udp_external_thread.join();
    udp_internal_thread.join();
}
