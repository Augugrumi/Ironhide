//
// Created by zanna on 05/10/18.
//

#include "egress.h"

void endpoint::Egress::manage_exiting_udp_packets(unsigned char* pkt,
                                                  size_t pkt_len) {
    // + 16 not considering sfc header
    auto headers = utils::PacketUtils::retrieve_ip_tcp_header(pkt + 16);
    client::udp::ClientUDP client;
    client::fd_type socket = client.send_only(
            pkt, pkt_len, const_cast<char*>(
                utils::PacketUtils::int_to_ip(headers.first.daddr).c_str()),
            headers.second.dest);
    ssize_t received_len;
    unsigned char * buffer = new unsigned char[BUFFER_SIZE];
    struct sockaddr_in server;
    server.sin_family      = AF_INET;
    server.sin_port        = headers.second.dest;
    server.sin_addr.s_addr = headers.first.daddr;
    socklen_t server_addr_len = sizeof(server);
    char* sfcid;
    char* next_ip;
    uint16_t next_port;
    unsigned char* formatted_pkt;
    do {
        received_len = recvfrom(socket,
                                buffer, BUFFER_SIZE,
                                0,
                                (struct sockaddr *)&server,
                                &(server_addr_len));
        if (received_len < 0) {
            perror("error receiving data");
            exit(EXIT_FAILURE);
        } else if (received_len > 0) {
            sfcid = classifier_.classify_pkt(buffer, received_len);
            // TODO check how to set ttl
            sfc_header flh =
                    utils::sfc_header::SFCUtilities::create_header(
                            atoi(sfcid), 0,const_cast<char*>(
                                    utils::PacketUtils::int_to_ip(
                                            headers.first.saddr).c_str()),
                            headers.second.source, const_cast<char*>(
                                    utils::PacketUtils::int_to_ip(
                                            headers.first.daddr).c_str()),
                            headers.second.dest, 1024, 0);
            // TODO set next_ip and next_port with call to roulette
            utils::sfc_header::SFCUtilities::prepend_header(buffer, pkt_len,
                                                            flh, formatted_pkt);
            client::udp::ClientUDP().send_and_wait_response(formatted_pkt,
                                                            pkt_len + 16,
                                                            next_ip, next_port);
        }
    } while(received_len > 0);

    free(pkt);
    free(buffer);

    // TODO think whe closing socket
}


void endpoint::Egress::manage_exiting_tcp_packets(unsigned char* pkt,
                                                  size_t pkt_len) {
    // + 16 not considering sfc header
    auto headers = utils::PacketUtils::retrieve_ip_tcp_header(pkt + 16);

    client::tcp::ClientTCP client;
    client.connect_to_server(
            const_cast<char*>(
                    utils::PacketUtils::int_to_ip(headers.first.daddr).c_str()),
            headers.second.dest);

    client::fd_type socket = client.access_to_socket();
    send(socket, pkt, pkt_len, 0);

    ssize_t received_len;
    unsigned char * buffer = new unsigned char[BUFFER_SIZE];
    char* sfcid;
    char* next_ip;
    uint16_t next_port;
    unsigned char* formatted_pkt;
    do {
        received_len = read(socket, buffer, BUFFER_SIZE);
        if (received_len < 0) {
            perror("error receiving data");
            exit(EXIT_FAILURE);
        } else if (received_len > 0) {
            sfcid = classifier_.classify_pkt(buffer, received_len);
            // TODO check how to set ttl
            sfc_header flh =
                    utils::sfc_header::SFCUtilities::create_header(
                            atoi(sfcid), 0,const_cast<char*>(
                                    utils::PacketUtils::int_to_ip(
                                            headers.first.saddr).c_str()),
                            headers.second.source, const_cast<char*>(
                                    utils::PacketUtils::int_to_ip(
                                            headers.first.daddr).c_str()),
                            headers.second.dest, 1024, 0);
            // TODO set next_ip and next_port with call to roulette
            utils::sfc_header::SFCUtilities::prepend_header(buffer, pkt_len,
                                                            flh, formatted_pkt);
            client::udp::ClientUDP().send_and_wait_response(formatted_pkt,
                                                            pkt_len + 16,
                                                            next_ip, next_port);
        }

    } while(received_len > 0);

    free(pkt);
    free(buffer);

    //close(new_socket_fd);
    // TODO think when closing socket
}

void endpoint::Egress::manage_pkt_from_chain(void * mngmnt_args) {

    // TODO integrate clients form communication calling
    // manage_exiting_udp_packets and manage_exiting_tcp_packets when needed

    auto args = (server::udp::udp_pkt_mngmnt_args *)mngmnt_args;

    std::cout << "bla" << std::endl;

    std::string ack = "ACK";
    sendto(args->socket_fd,
           ack.c_str(),
           ack.length(),
           0,
           reinterpret_cast<struct sockaddr*>(&args->client_address),
           sizeof(args->client_address));


    // TODO check if TCP or UDP
    sfc_header flh = utils::sfc_header::SFCUtilities::retrieve_header(
            (unsigned char*)args->pkt);

    auto headers =
            utils::PacketUtils::retrieve_ip_udp_header(
                    (unsigned char*)(args->pkt + 16));
    // TODO change with update entry
    add_entry(ConnectionEntry(
            utils::PacketUtils::int_to_ip(headers.first.saddr),
            utils::PacketUtils::int_to_ip(headers.first.daddr),
            headers.second.source, headers.second.dest,
            std::to_string(flh.p_id)),
              args->socket_fd,
              Protocol::UDP); // TODO change based on protocol

    if (true) {
        std::function<void ()> f = [this, &args]() {
            manage_exiting_tcp_packets((unsigned char*)args->pkt, args->pkt_len);
        };
        ASYNC_TASK(f);
    } else {
        std::function<void ()> f = [this, &args]() {
            manage_exiting_udp_packets((unsigned char*)args->pkt, args->pkt_len);
        };
        ASYNC_TASK(f);
    };

}

void endpoint::Egress::start(uint16_t int_port, uint16_t ext_port) {
    std::function<void (void*)> vnf_callback = [this](void* args) {
        this->manage_pkt_from_chain(args);
    };
    std::thread udp_internal_thread([&int_port, &vnf_callback]{
        auto server = new server::udp::ServerUDP(int_port);
        server->set_pkt_manager(vnf_callback);
        server->run();
    });

    udp_internal_thread.join();
}