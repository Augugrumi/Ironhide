//
// Created by zanna on 04/10/18.
//

#include "serverudp.h"

server::udp::ServerUDP::ServerUDP(uint16_t port) : Server(port){}

void server::udp::ServerUDP::run() {
    LOG(ldebug, "running server udp");
    int socket_fd;
    struct sockaddr_in address;
    udp_pkt_mngmnt_args * args;
    socklen_t client_address_len;

    /* Initialise IPv4 address. */
    memset(&address, 0, sizeof address);
    address.sin_family = AF_INET;
    address.sin_port = htons(port_);
    address.sin_addr.s_addr = INADDR_ANY;

    /* Create udp socket. */
    if ((socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    /* Bind address to socket. */
    if (bind(socket_fd, (struct sockaddr *)&address, sizeof address) == -1) {
        perror("bind");
        exit(1);
    }

    setup_sign_catching();

    while (!stopped_) {
        /* Create pthread argument for each connection to client. */
        /* TODO: malloc'ing before accepting a connection causes only one small
         * memory when the program exits. It can be safely ignored.
         */
        args = (udp_pkt_mngmnt_args*)malloc(sizeof *args);
        if (!args) {
            perror("malloc");
            continue;
        }

        /* Accept connection to client. */
        client_address_len = sizeof args->client_address;
        args->pkt = new char[BUFFER_SIZE];
        ssize_t pkt_len = recvfrom(socket_fd,
                                   args->pkt,
                                   BUFFER_SIZE, 0,
                                   (struct sockaddr *)&args->client_address,
                                   &client_address_len);
        LOG(ldebug, "Received packet");
        if (pkt_len == -1) {
            perror("accept");
            free(args);
            continue;
        }
        args->pkt_len = pkt_len;
        args->socket_fd = socket_fd;
        /* Initialise pthread argument. */
        //pthread_arg->new_socket_fd = new_socket_fd;
        /* TODO: Initialise arguments passed to threads here. See lines 22 and
         * 139.
         */

        /* Create thread to serve connection to client. */
        ASYNC_TASK(
                std::bind<void>(manager_, args));
    }

    /* close(socket_fd);
     * TODO: If you really want to close the socket, you would do it in
     * signal_handler(), meaning socket_fd would need to be a global variable.
     */
}

