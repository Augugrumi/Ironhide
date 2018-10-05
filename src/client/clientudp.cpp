//
// Created by zanna on 05/10/18.
//


#include "clientudp.h"

void client::udp::ClientUDP::send_and_wait_response(unsigned char *message,
                                                    size_t message_len,
                                                    char *dst, uint16_t port) {

    struct addrinfo hints;
    struct addrinfo *result, *rp;
    fd_type sfd;
    int s;
    ssize_t res = -1;
    bool send_flag = true;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
    hints.ai_flags = 0;
    hints.ai_protocol = IPPROTO_UDP; /* UDP protocol */

    s = getaddrinfo(dst, std::to_string(port).c_str(), &hints, &result);

    if (s != 0) {
        perror("Error getting info for destination");
        exit(EXIT_FAILURE);
    }

    for (rp = result; rp != nullptr && send_flag; rp = rp->ai_next) {

        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd > 0 && connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1) {
            res = sendto(
                    sfd,
                    message,
                    message_len,
                    MSG_CONFIRM,
                    rp->ai_addr,
                    rp->ai_addrlen);
            if (res > 0) {
                send_flag = false;
            }
        }
    }

    char buffer[BUFFER_SIZE];
    res = recvfrom(sfd,
                   (char *)buffer, BUFFER_SIZE,
                   MSG_WAITALL,
                   rp->ai_addr, &(rp->ai_addrlen));

    close(sfd);
    freeaddrinfo(result);


/*


    fd_type sockfd;

    char buffer[BUFFER_SIZE];
    struct sockaddr_in servaddr;

    ssize_t response_len;

    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = INADDR_ANY;

    sendto(sockfd, message, message_len,
           MSG_CONFIRM, (const struct sockaddr *) &servaddr,
           sizeof(servaddr));

    socklen_t addr_len = sizeof(servaddr);

    response_len = recvfrom(sockfd, (char *)buffer, BUFFER_SIZE,
                            MSG_WAITALL, (struct sockaddr *) &servaddr,
                            &addr_len);

    close(sockfd);*/
}