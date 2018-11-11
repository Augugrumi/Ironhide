#include "clientudp.h"

void client::udp::ClientUDP::send_and_wait_response(unsigned char *message,
                                                    size_t message_len,
                                                    const char *dst,
                                                    uint16_t port) {

    struct addrinfo hints{};
    struct addrinfo *result, *rp;
    fd_type sfd = -1;
    int s;
    ssize_t res;
    bool send_flag = true;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
    hints.ai_flags = 0;
    hints.ai_protocol = IPPROTO_UDP; /* UDP protocol */

    LOG(ldebug, "addr " + std::string(dst));
    LOG(ldebug, "port " + std::to_string(port));

    s = getaddrinfo(dst, std::to_string(port).c_str(), &hints, &result);

    if (s != 0) {
        LOG(lfatal, "Error getting info for destination");
        if (s == EAI_SYSTEM) {
            LOG(lfatal, std::string("getaddrinfo: ") +
                        std::string(strerror(errno)));
        } else {
            LOG(lfatal, std::string("getaddrinfo: ") +
                        std::string(gai_strerror(s)));
        }
        exit(EXIT_FAILURE);
    }

    for (rp = result; send_flag && rp != nullptr; rp = rp->ai_next) {

        if (sfd > 0) {
            LOG(ltrace, "Close ln 41 clientudp.cpp");
            close(sfd);
        }
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd > 0 && connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1) {
            LOG(ldebug, "sendto inside clientudp " + std::to_string(sfd));
            res = sendto(
                    sfd,
                    message,
                    message_len,
                    0,
                    rp->ai_addr,
                    rp->ai_addrlen);
            if (res > 0) {
                send_flag = false;
                LOG(lfatal, "Sent " + std::to_string(res));
            } else {
                LOG(lfatal, "Error " + std::to_string(res));
                perror("send error");
            }
            res = recvfrom(sfd, message, message_len, 0, rp->ai_addr, &rp->ai_addrlen);
        }
    }



    if (sfd > 0) {
        LOG(ltrace, "Close ln 62 clientudp.cpp");
        close(sfd);
    }
    freeaddrinfo(result);
}

std::pair<int, struct addrinfo> client::udp::ClientUDP::send_only(unsigned char *message,
                                                  size_t message_len,
                                                  const char *dst,
                                                  uint16_t port) {
    /*struct addrinfo hints{};
    struct addrinfo *result, *rp;
    fd_type sfd = -1;
    int s;
    ssize_t res;
    bool send_flag = true;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = 0;
    hints.ai_protocol = IPPROTO_UDP;

    s = getaddrinfo(dst, std::to_string(port).c_str(), &hints, &result);

    if (s != 0) {
        perror("Error getting info for destination");
        exit(EXIT_FAILURE);
    }

    for (rp = result; rp != nullptr && send_flag; rp = rp->ai_next) {

        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        int option = 1;
        setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
        if (sfd > 0 && connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1) {
            res = sendto(
                    sfd,
                    message,
                    message_len,
                    0,
                    rp->ai_addr,
                    rp->ai_addrlen);
            if (res > 0) {
                send_flag = false;
            }
        }
    }

    return sfd;*/

    struct addrinfo hints{};
    struct addrinfo *result, *rp;
    fd_type sfd = -1;
    int s;
    ssize_t res;
    bool send_flag = true;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
    hints.ai_flags = 0;
    hints.ai_protocol = IPPROTO_UDP; /* UDP protocol */

    LOG(ldebug, "addr " + std::string(dst));
    LOG(ldebug, "port " + std::to_string(port));

    s = getaddrinfo(dst, std::to_string(port).c_str(), &hints, &result);

    if (s != 0) {
        LOG(lfatal, "Error getting info for destination");
        if (s == EAI_SYSTEM) {
            LOG(lfatal, std::string("getaddrinfo: ") +
                        std::string(strerror(errno)));
        } else {
            LOG(lfatal, std::string("getaddrinfo: ") +
                        std::string(gai_strerror(s)));
        }
        exit(EXIT_FAILURE);
    }
    struct addrinfo to_ret;
    for (rp = result; send_flag && rp != nullptr; rp = rp->ai_next) {

        if (sfd > 0) {
            LOG(ltrace, "Close ln 41 clientudp.cpp");
            close(sfd);
        }
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd > 0 && connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1) {
            LOG(ldebug, "sendto inside clientudp " + std::to_string(sfd));
            res = sendto(
                    sfd,
                    message,
                    message_len,
                    0,
                    rp->ai_addr,
                    rp->ai_addrlen);
            if (res > 0) {
                send_flag = false;
                to_ret = *rp;
                LOG(lfatal, "Sent " + std::to_string(res));
            } else {
                LOG(lfatal, "Error " + std::to_string(res));
                LOG(lfatal, std::to_string(message_len));
                perror("send error");
            }
        }
    }

    /*if (sfd > 0) {
        LOG(ltrace, "Close ln 62 clientudp.cpp");
        close(sfd);
    }
    freeaddrinfo(result);
    */

    return std::pair<fd_type, struct addrinfo>(sfd,to_ret);
}
