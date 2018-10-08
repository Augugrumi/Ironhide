//
// Created by zanna on 05/10/18.
//

#include "ingress.h"

void endpoint::Ingress::manage_entering_tcp_packets(void * mngmnt_args) {
    auto args = (server::tcp::tcp_pkt_mngmnt_args*)mngmnt_args;
    int new_socket_fd = args->new_socket_fd;

    free(args);

    std::cout << "bla" << std::endl;

    /* TODO: Put client interaction code here. For example, use
     * write(new_socket_fd,,) and read(new_socket_fd,,) to send and receive
     * messages with the client.
     */

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

    delete(args->pkt);
    free(args);
    /* TODO: Put client interaction code here. For example, use
     * write(new_socket_fd,,) and read(new_socket_fd,,) to send and receive
     * messages with the client.
     */
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

    delete(args->pkt);
    free(args);
    /* TODO: Put client interaction code here. For example, use
     * write(new_socket_fd,,) and read(new_socket_fd,,) to send and receive
     * messages with the client.
     */
}

void endpoint::Ingress::start(uint16_t int_port, uint16_t ext_port) {
    std::thread udp_internal_thread([&int_port]{
        auto server = new server::udp::ServerUDP(int_port);
        server->set_pkt_manager(Ingress::manage_pkt_from_chain);
        server->run();
    });

    std::thread udp_external_thread([&ext_port]{
        auto server = new server::udp::ServerUDP(ext_port);
        server->set_pkt_manager(Ingress::manage_entering_udp_packets);
        server->run();
    });
    std::thread tcp_thread([&ext_port]{
        auto server = new server::tcp::ServerTCP(ext_port);
        server->set_pkt_manager(Ingress::manage_entering_tcp_packets);
        server->run();
    });

    tcp_thread.join();
    udp_external_thread.join();
    udp_internal_thread.join();
}
