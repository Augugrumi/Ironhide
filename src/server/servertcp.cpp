#include "servertcp.h"

server::tcp::ServerTCP::ServerTCP(uint16_t port) : Server(port) {}

void server::tcp::ServerTCP::run() {
    bool run_flag = true;
    struct sockaddr_in addr;
    tcp_pkt_mngmnt_args *args;
    socklen_t addr_size;
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port_);
    addr.sin_addr.s_addr = INADDR_ANY;
    addr_size = sizeof(addr);

    if (fd < 0) {
        LOG(lfatal, "Impossible to obtain a valid file descriptor");
    }

    int option = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    if (bind(fd, reinterpret_cast<struct sockaddr*>(&addr), addr_size) < 0 ) {
        LOG(lfatal, "Faliure binding to port: " + std::to_string(port_));
        exit(EXIT_FAILURE);
    }

    if (listen(fd, BACKLOG) < 0) {
        LOG(lfatal, "Error while listening int the desired port");
        exit(EXIT_FAILURE);
    }

    setup_sign_catching();

    addr_size = sizeof(addr);
    int incoming_connection_fd = accept(fd,
                                        reinterpret_cast<struct sockaddr*>(&addr),
                                        &addr_size);

    args = (tcp_pkt_mngmnt_args *) malloc(sizeof *args);
    if (!args) {
        perror("malloc");
    }

    auto handle_connection = [this, args](int icfd) {
        struct pollfd pollfd;
        pollfd.fd = icfd;
        pollfd.events = POLLIN;
        pollfd.revents = 0;

        args->new_socket_fd = icfd;
        auto recv_elaborator = std::bind<void>(manager_, args);

        if (pollfd.fd < 0) {
            LOG(lfatal, "Impossible to obtain a valid file descriptor for the incoming connection");
        }

        while (true) {
            pollfd.fd = icfd;
            pollfd.events = POLLIN;
            pollfd.revents = 0;

            LOG(ltrace, "Iteration");

            int poll_ret = poll(&pollfd, 1, -1);
            if (poll_ret > 0) {
                if (pollfd.revents & POLLIN) {
                    LOG(ltrace, "Detected POLLIN event");
                    recv_elaborator();
                }
                if (pollfd.revents & POLLPRI) {
                    LOG(ltrace, "2");
                }
                if (pollfd.revents & POLLOUT) {
                    LOG(ltrace, "3");
                }
                if (pollfd.revents & POLLRDHUP) {
                    LOG(ltrace, "4");
                }
                if (pollfd.revents & POLLERR) {
                    LOG(ltrace, "5");
                }
                if (pollfd.revents & POLLHUP) {
                    LOG(ltrace, "6");
                }
                if (pollfd.revents & POLLNVAL) {
                    LOG(ltrace, "7");
                } else {
                    LOG(ltrace, "Unknow event: " + std::to_string(pollfd.events));
                }
            } else {
                LOG(lwarn, "poll_ret value: " + std::to_string(poll_ret));
                break;
            }
        }
    };

    GO_ASYNC(std::bind<void>(handle_connection, incoming_connection_fd));
}

/*
void server::tcp::ServerTCP::run() {
    stopped_ = false;
    int socket_fd, new_socket_fd;
    struct sockaddr_in address{};
    tcp_pkt_mngmnt_args *args;
    socklen_t client_address_len;

    // Initialise IPv4 address.
    memset(&address, 0, sizeof address);
    address.sin_family = AF_INET;
    address.sin_port = htons(port_);
    address.sin_addr.s_addr = INADDR_ANY;

    // Create TCP socket.
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    int option = 1;
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    // Bind address to socket.
    if (bind(socket_fd, (struct sockaddr *) &address, sizeof address) == -1) {
        perror("bind");
        exit(1);
    }

    // Listen on socket.
    if (listen(socket_fd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }
    setup_sign_catching();
    while (!stopped_) {
        /* Create pthread argument for each connection to client. */
        /* TODO: malloc'ing before accepting a connection causes only one small
         * memory when the program exits. It can be safely ignored.
         *//*
        args = (tcp_pkt_mngmnt_args *) malloc(sizeof *args);
        if (!args) {
            perror("malloc");
            continue;
        }

        // Accept connection to client.
        client_address_len = sizeof args->client_address;
        new_socket_fd = accept(socket_fd,
                               (struct sockaddr *) &args->client_address,
                               &client_address_len);

        if (new_socket_fd == -1) {
            perror("accept");
            free(args);
            continue;
        }

        // Initialise pthread argument.
        args->new_socket_fd = new_socket_fd;

        GO_ASYNC(std::bind<void>(manager_, args));
    }

    /* close(socket_fd);
     * TODO: If you really want to close the socket, you would do it in
     * signal_handler(), meaning socket_fd would need to be a global variable.
     *//*
}
*/
