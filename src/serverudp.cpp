//
// Created by zanna on 04/10/18.
//

#include "serverudp.h"

server::udp::ServerUDP::ServerUDP(uint16_t port) : Server(port){}

void server::udp::ServerUDP::run() {
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

    /* Assign signal handlers to signals. */
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
        perror("signal");
        exit(1);
    }
    if (signal(SIGTERM, Server::signal_handler) == SIG_ERR) {
        perror("signal");
        exit(1);
    }
    if (signal(SIGINT, Server::signal_handler) == SIG_ERR) {
        perror("signal");
        exit(1);
    }

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
        if (pkt_len == -1) {
            perror("accept");
            free(args);
            continue;
        }
        args->socket_fd = socket_fd;
        /* Initialise pthread argument. */
        //pthread_arg->new_socket_fd = new_socket_fd;
        /* TODO: Initialise arguments passed to threads here. See lines 22 and
         * 139.
         */

        /* Create thread to serve connection to client. */
        ASYNC_TASK(
                std::bind<void>(&ServerUDP::pkt_mngmnt, this, args));
    }

    /* close(socket_fd);
     * TODO: If you really want to close the socket, you would do it in
     * signal_handler(), meaning socket_fd would need to be a global variable.
     */
}

void server::udp::ServerUDP::pkt_mngmnt(void* mngmnt_args) {
    auto args = (udp_pkt_mngmnt_args *)mngmnt_args;
    //struct sockaddr_in client_address = pthread_arg->client_address;
    /* TODO: Get arguments passed to threads here. See lines 22 and 116. */


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
