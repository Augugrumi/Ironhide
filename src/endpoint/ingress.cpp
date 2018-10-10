//
// Created by zanna on 05/10/18.
//

#include "ingress.h"

void endpoint::Ingress::manage_entering_tcp_packets(void * mngmnt_args) {
    auto args = (server::tcp::tcp_pkt_mngmnt_args*)mngmnt_args;
    int new_socket_fd = args->new_socket_fd;

    std::cout << "bla" << std::endl;

    ssize_t read_size;
    unsigned char pkt[BUFFER_SIZE];
    char* sfcid, * prev_sfcid = const_cast<char*>("");
    bool first_pkt = true;
    while((read_size = recv(new_socket_fd, pkt, BUFFER_SIZE, 0)) > 0) {
        sfcid = Endpoint::classifier_.classify_pkt((unsigned char*) pkt, read_size);

        if (first_pkt || (strcmp(sfcid, prev_sfcid) != 0)) {
            auto headers =
                    utils::PacketUtils::retrieve_ip_tcp_header((unsigned char*)pkt);

            add_entry(ConnectionEntry(
                            utils::PacketUtils::int_to_ip(headers.first.saddr),
                            utils::PacketUtils::int_to_ip(headers.first.daddr),
                            headers.second.source, headers.second.dest, sfcid),
                      new_socket_fd,
                      Protocol::TCP);


            // TODO check how to set ttl
            sfc_header flh =
                    utils::sfc_header::SFCUtilities::create_header(
                            atoi(sfcid), 0,const_cast<char*>(
                                    utils::PacketUtils::int_to_ip(
                                            headers.first.saddr).c_str()),
                            headers.second.source,const_cast<char*>(
                                    utils::PacketUtils::int_to_ip(
                                            headers.first.daddr).c_str()),
                            headers.second.dest, 1024, 0);
            char* next_ip;
            uint16_t next_port;
            client::udp::ClientUDP().send_and_wait_response(pkt, read_size, next_ip, next_port);

            prev_sfcid = sfcid;
            first_pkt = false;
        }

        //TODO retrieve next hop from db

        //Send the message in to the vnf
        //write(client_sock , client_message , strlen(client_message));
    }

    if(read_size == 0) {
        puts("Client disconnected");
        fflush(stdout);
    }
    else if(read_size == -1) {
        perror("recv failed");
    }

    free(args);

    //close(new_socket_fd);
}


void endpoint::Ingress::manage_entering_udp_packets(void * mngmnt_args) {
    auto args = (server::udp::udp_pkt_mngmnt_args *)mngmnt_args;


    std::cout << "bla" << std::endl;

    std::string ack = "ACK";
    sendto(args->socket_fd,
           ack.c_str(),
           ack.length(),
           0,
           reinterpret_cast<struct sockaddr*>(&args->client_address),
           sizeof(args->client_address));

    char* sfcid = Endpoint::classifier_.classify_pkt((unsigned char*) args->pkt,
                                           args->pkt_len);

    auto headers =
            utils::PacketUtils::retrieve_ip_udp_header(
                    (unsigned char*)args->pkt);
    add_entry(ConnectionEntry(
                utils::PacketUtils::int_to_ip(headers.first.saddr),
                utils::PacketUtils::int_to_ip(headers.first.daddr),
                headers.second.source, headers.second.dest, sfcid),
                   args->socket_fd,
                   Protocol::UDP);

    // TODO check how to set ttl
    sfc_header flh =
            utils::sfc_header::SFCUtilities::create_header(atoi(sfcid), 0,
                    const_cast<char*>(utils::PacketUtils::int_to_ip(
                            headers.first.saddr).c_str()),
                    headers.second.source,
                    const_cast<char*>(utils::PacketUtils::int_to_ip(
                            headers.first.daddr).c_str()),
                    headers.second.dest, 1024, 0);

    char* next_ip;
    uint16_t next_port;
    unsigned char* formatted_pkt;
    unsigned char* p = (unsigned char*)args->pkt;
    utils::sfc_header::SFCUtilities::prepend_header(p, args->pkt_len,
                                                    flh, formatted_pkt);
    client::udp::ClientUDP().send_and_wait_response(formatted_pkt,
            args->pkt_len + 16, next_ip, next_port);


    delete(args->pkt);
    free(args);
}

void endpoint::Ingress::manage_pkt_from_chain(void * mngmnt_args) {
    auto args = (server::udp::udp_pkt_mngmnt_args *)mngmnt_args;

    std::cout << "bla" << std::endl;

    std::string ack = "ACK";
    sendto(args->socket_fd,
           ack.c_str(),
           ack.length(),
           0,
           reinterpret_cast<struct sockaddr*>(&args->client_address),
           sizeof(args->client_address));


    struct sfc_header header =
            utils::sfc_header::SFCUtilities::retrieve_header((uint8_t*)args->pkt);
    unsigned char* payload;
    utils::sfc_header::SFCUtilities::retrieve_payload((uint8_t*)args->pkt,
                                                      args->pkt_len,
                                                      payload);

    endpoint::socket_fd sock = retrieve_connection(ConnectionEntry(
            utils::sfc_header::SFCUtilities::int_to_ip(header.source_address),
            utils::sfc_header::SFCUtilities::int_to_ip(header.destination_address),
            header.source_port, header.destination_port,
            std::to_string(header.p_id)));

    // TODO check if TCP or UDP
    unsigned char resp_buff[BUFFER_SIZE];
    ssize_t resp_c;
    if (false) {
        send(sock , args->pkt, args->pkt_len, 0);
        resp_c = read( sock , resp_buff, BUFFER_SIZE);
        if (resp_c < -1) {
            perror("error receiving data");
            exit(EXIT_FAILURE);
        }
    } else {
        struct sockaddr_in server;
        server.sin_family      = AF_INET;
        server.sin_port        = header.destination_port;
        server.sin_addr.s_addr = header.destination_address;
        socklen_t s = sizeof(server);
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
