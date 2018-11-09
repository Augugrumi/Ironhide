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

    int on = 1;
    int rc = ioctl(fd, FIONBIO, (char *)&on);
    if (rc < 0)
    {
        perror("ioctl() failed");
        close(fd);
        exit(-1);
    }

    if (bind(fd, reinterpret_cast<struct sockaddr*>(&addr), addr_size) < 0 ) {
        LOG(lfatal, "Faliure binding to port: " + std::to_string(port_));
        exit(EXIT_FAILURE);
    }

    if (listen(fd, BACKLOG) < 0) {
        LOG(lfatal, "Error while listening int the desired port");
        exit(EXIT_FAILURE);
    }

    setup_sign_catching();

    struct pollfd fds[200];
    memset(fds, 0 , sizeof(fds));

    fds[0].fd = fd;
    fds[0].events = POLLIN;

    char buffer[BUFFER_SIZE];
    int nfds = 1, current_size, i, new_sd, len, j;
    bool end_server, close_conn, compress_array = false;
    do {
        printf("Waiting on poll()...\n");
        rc = poll(fds, nfds, -1);
        if (rc < 0) {
            perror("  poll() failed");
            break;
        }

        current_size = nfds;
        for (i = 0; i < current_size; i++) {
            /*********************************************************/
            /* Loop through to find the descriptors that returned    */
            /* POLLIN and determine whether it's the listening       */
            /* or the active connection.                             */
            /*********************************************************/
            if(fds[i].revents == 0)
                continue;

            /*********************************************************/
            /* If revents is not POLLIN, it's an unexpected result,  */
            /* log and end the server.                               */
            /*********************************************************/
            if(fds[i].revents != POLLIN)
            {
                printf("  Error! revents = %d\n", fds[i].revents);
                end_server = true;
                break;

            }
            if (fds[i].fd == fd)
            {
                /*******************************************************/
                /* Listening descriptor is readable.                   */
                /*******************************************************/
                printf("  Listening socket is readable\n");

                /*******************************************************/
                /* Accept all incoming connections that are            */
                /* queued up on the listening socket before we         */
                /* loop back and call poll again.                      */
                /*******************************************************/
                do
                {
                    /*****************************************************/
                    /* Accept each incoming connection. If               */
                    /* accept fails with EWOULDBLOCK, then we            */
                    /* have accepted all of them. Any other              */
                    /* failure on accept will cause us to end the        */
                    /* server.                                           */
                    /*****************************************************/
                    new_sd = accept(fd, NULL, NULL);
                    if (new_sd < 0)
                    {
                        if (errno != EWOULDBLOCK)
                        {
                            perror("  accept() failed");
                            end_server = true;
                        }
                        break;
                    }

                    /*****************************************************/
                    /* Add the new incoming connection to the            */
                    /* pollfd structure                                  */
                    /*****************************************************/
                    printf("  New incoming connection - %d\n", new_sd);
                    fds[nfds].fd = new_sd;
                    fds[nfds].events = POLLIN;
                    nfds++;

                    /*****************************************************/
                    /* Loop back up and accept another incoming          */
                    /* connection                                        */
                    /*****************************************************/
                } while (new_sd != -1);
            }

                /*********************************************************/
                /* This is not the listening socket, therefore an        */
                /* existing connection must be readable                  */
                /*********************************************************/

            else {
                printf("  Descriptor %d is readable\n", fds[i].fd);
                close_conn = false;
                /*******************************************************/
                /* Receive all incoming data on this socket            */
                /* before we loop back and call poll again.            */
                /*******************************************************/
                do
                {
                    args = new tcp_pkt_mngmnt_args();
                    args->pkt = new char[BUFFER_SIZE];
                    args->new_socket_fd = fds[i].fd;
                    /*****************************************************/
                    /* Receive data on this connection until the         */
                    /* recv fails with EWOULDBLOCK. If any other         */
                    /* failure occurs, we will close the                 */
                    /* connection.                                       */
                    /*****************************************************/
                    rc = recv(fds[i].fd, args->pkt, BUFFER_SIZE, 0);
                    if (rc < 0)
                    {
                        if (errno != EWOULDBLOCK)
                        {
                            perror("  recv() failed");
                            close_conn = true;
                        }
                        break;
                    }

                    /*****************************************************/
                    /* Check to see if the connection has been           */
                    /* closed by the client                              */
                    /*****************************************************/
                    if (rc == 0)
                    {
                        printf("  Connection closed\n");
                        close_conn = true;
                        break;
                    }

                    args->pkt_size = rc;
                    GO_ASYNC(std::bind<void>(manager_, args));
                } while(true);

                /*******************************************************/
                /* If the close_conn flag was turned on, we need       */
                /* to clean up this active connection. This            */
                /* clean up process includes removing the              */
                /* descriptor.                                         */
                /*******************************************************/
                if (close_conn)
                {
                    close(fds[i].fd);
                    fds[i].fd = -1;
                    compress_array = true;
                }


            }  /* End of existing connection is readable             */
        } /* End of loop through pollable descriptors              */

        /***********************************************************/
        /* If the compress_array flag was turned on, we need       */
        /* to squeeze together the array and decrement the number  */
        /* of file descriptors. We do not need to move back the    */
        /* events and revents fields because the events will always*/
        /* be POLLIN in this case, and revents is output.          */
        /***********************************************************/
        if (compress_array) {
            compress_array = false;
            for (i = 0; i < nfds; i++)
            {
                if (fds[i].fd == -1)
                {
                    for(j = i; j < nfds; j++)
                    {
                        fds[j].fd = fds[j+1].fd;
                    }
                    i--;
                    nfds--;
                }
            }
        }

    } while (end_server == false);

    /*addr_size = sizeof(addr);
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
        args->first_run = true;
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
                    args->first_run = false;
                    continue;
                }
                if (pollfd.revents & POLLPRI) {
                    LOG(ltrace, "2");
                    continue;
                }
                if (pollfd.revents & POLLOUT) {
                    LOG(ltrace, "3");
                    continue;
                }
                if (pollfd.revents & POLLRDHUP) {
                    LOG(ltrace, "4");
                    continue;
                }
                if (pollfd.revents & POLLERR) {
                    LOG(ltrace, "5");
                    continue;
                }
                if (pollfd.revents & POLLHUP) {
                    LOG(ltrace, "6");
                    continue;
                }
                if (pollfd.revents & POLLNVAL) {
                    LOG(ltrace, "7");
                    exit(EXIT_FAILURE);
                } else {
                    LOG(ltrace, "Unknow event: " + std::to_string(pollfd.revents));
                    continue;
                }
            } else {
                LOG(lwarn, "poll_ret value: " + std::to_string(poll_ret));
                break;
            }
        }
    };

    GO_ASYNC(std::bind<void>(handle_connection, incoming_connection_fd));*/
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
