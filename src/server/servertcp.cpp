//
// Created by zanna on 04/10/18.
//

#include "servertcp.h"

server::tcp::ServerTCP::ServerTCP(uint16_t port) : Server(port){}

void server::tcp::ServerTCP::run() {
    stopped_ = false;
    int socket_fd, new_socket_fd;
    struct sockaddr_in address;
    tcp_pkt_mngmnt_args* args;
    socklen_t client_address_len;

    /* Initialise IPv4 address. */
    memset(&address, 0, sizeof address);
    address.sin_family = AF_INET;
    address.sin_port = htons(port_);
    address.sin_addr.s_addr = INADDR_ANY;

    /* Create TCP socket. */
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    /* Bind address to socket. */
    if (bind(socket_fd, (struct sockaddr *)&address, sizeof address) == -1) {
        perror("bind");
        exit(1);
    }

    /* Listen on socket. */
    if (listen(socket_fd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    setup_sign_catching();

    while (!stopped_) {
        /* Create pthread argument for each connection to client. */
        /* TODO: malloc'ing before accepting a connection causes only one small
         * memory when the program exits. It can be safely ignored.
         */
        args = (tcp_pkt_mngmnt_args*)malloc(sizeof *args);
        if (!args) {
            perror("malloc");
            continue;
        }

        /* Accept connection to client. */
        client_address_len = sizeof args->client_address;
        new_socket_fd = accept(socket_fd, (struct sockaddr *)&args->client_address, &client_address_len);
        if (new_socket_fd == -1) {
            perror("accept");
            free(args);
            continue;
        }

        /* Initialise pthread argument. */
        args->new_socket_fd = new_socket_fd;
        /* TODO: Initialise arguments passed to threads here. See lines 22 and
         * 139.
         */

        /* Create thread to serve connection to client. */
        ASYNC_TASK(
                std::bind<void>(&ServerTCP::pkt_mngmnt, this, args));
    }

    /* close(socket_fd);
     * TODO: If you really want to close the socket, you would do it in
     * signal_handler(), meaning socket_fd would need to be a global variable.
     */
}

void server::tcp::ServerTCP::pkt_mngmnt(void* mngmnt_args) {
    auto args = (tcp_pkt_mngmnt_args*)mngmnt_args;
    int new_socket_fd = args->new_socket_fd;
    //struct sockaddr_in client_address = pthread_arg->client_address;
    /* TODO: Get arguments passed to threads here. See lines 22 and 116. */

    free(args);

    std::cout << "bla" << std::endl;

    /* TODO: Put client interaction code here. For example, use
     * write(new_socket_fd,,) and read(new_socket_fd,,) to send and receive
     * messages with the client.
     */

    close(new_socket_fd);
}
