#include <poll.h>
#include "serverudp.h"

server::udp::ServerUDP::ServerUDP(uint16_t port) : Server(port) {}

void server::udp::ServerUDP::run() {
    /*stopped_ = false;
    int socket_fd;
    struct sockaddr_in address{};
    udp_pkt_mngmnt_args *args;
    socklen_t client_address_len;

    memset(&address, 0, sizeof address);
    address.sin_family = AF_INET;
    address.sin_port = htons(port_);
    address.sin_addr.s_addr = INADDR_ANY;

    if ((socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    int option = 1;
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    if (bind(socket_fd, (struct sockaddr *) &address, sizeof address) == -1) {
        perror("bind");
        exit(1);
    }

    setup_sign_catching();

    while (!stopped_) {
        args = (udp_pkt_mngmnt_args *) malloc(sizeof *args);
        if (!args) {
            perror("malloc");
            continue;
        }

        client_address_len = sizeof args->client_address;
        args->pkt = new char[BUFFER_SIZE];
        ssize_t pkt_len = recvfrom(socket_fd,
                                   args->pkt,
                                   BUFFER_SIZE, 0,
                                   (struct sockaddr *) &args->client_address,
                                   &client_address_len);
        LOG(lfatal, std::to_string(pkt_len));
        LOG(ldebug, "Received packet");
        if (pkt_len == -1) {
            perror("accept");
            free(args);
            continue;
        }
        args->pkt_len = pkt_len;
        args->socket_fd = socket_fd;

        GO_ASYNC(std::bind<void>(manager_, args));
    }*/

    /* close(socket_fd);
     * TODO: If you really want to close the socket, you would do it in
     * signal_handler(), meaning socket_fd would need to be a global variable.
     */

    struct pollfd pollfd;
    struct sockaddr_in addr;

    char* buf;
    buf = new char[BUFFER_SIZE];

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port_);

    stopped_ = false;

    int fd = socket(AF_INET, SOCK_DGRAM, 0);

    // Setting events for incoming data
    pollfd.fd = fd;
    pollfd.events = POLLIN;
    pollfd.revents = 0;


    if (pollfd.fd < 0) {
        LOG(lfatal, "Impossible to obtain a valid file descriptor");
        exit(1);
    }

    if (bind(pollfd.fd,
             reinterpret_cast<struct sockaddr*>(&addr),
             sizeof(addr)) < 0) {
        LOG(lfatal, "Faliure binding to port: " + std::to_string(port_));
        exit(1);
    }

    // Main loop
    while (!stopped_) {
        /*
         * The first parameter, ufds, must point to an array of struct pollfd.
         * Each element in the array specifies a file descriptor that the
         * program is interested in monitoring, and what events on that file
         * descriptor the program would like to know about. The next parameter,
         * nfds, tells the kernel how many total items are in the ufds array.
         * The final parameter, timeout, is the maximum time, in milliseconds,
         * that the kernel should wait for the activities the ufds array
         * specifies. If timeout contains a negative value, the kernel will wait
         * forever. If nfds is zero, poll() becomes a simple millisecond sleep.
         */

        pollfd.fd = fd;
        pollfd.events = POLLIN;
        pollfd.revents = 0;

        int poll_ret = poll(&pollfd, 1, -1);
        if (poll_ret > 0) {
            if (pollfd.revents & POLLIN) {
                LOG(ltrace, "Detected POLLIN event");


                udp_pkt_mngmnt_args* args = new udp_pkt_mngmnt_args();
                socklen_t client_address_len = sizeof(args->client_address);
                args->pkt = new char[BUFFER_SIZE];
                ssize_t pkt_len = recvfrom(pollfd.fd,
                                           args->pkt,
                                           BUFFER_SIZE, 0,
                                           (struct sockaddr *) &args->client_address,
                                           &client_address_len);
                LOG(lfatal, std::to_string(pkt_len));
                LOG(ldebug, "Received packet");
                if (pkt_len == -1) {
                    perror("accept");
                    free(args);
                    continue;
                }
                args->pkt_len = pkt_len;
                args->socket_fd = pollfd.fd;

                ASYNC_TASK(std::bind<void>(manager_, args));
            }
        } else {
            LOG(lfatal, "Error fetching data in poll()");
        }
    }
}

