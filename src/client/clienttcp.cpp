//
// Created by zanna on 05/10/18.
//

#include "clienttcp.h"

void client::tcp::ClientTCP::connect_to_server(const char* dst, uint16_t port) {
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket creation error");
        exit(EXIT_FAILURE);
    }
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    fd_type sfd = - 1;
    int s;
    ssize_t res = -1;
    bool send_flag = true;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM; /* Stream socket */
    hints.ai_flags = 0;
    hints.ai_protocol = IPPROTO_TCP; /* TCP protocol */

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, dst, &serv_addr.sin_addr)<=0) {
        perror("invalid address / address not supported");
        exit(EXIT_FAILURE);
    }

    s = getaddrinfo(dst, std::to_string(port).c_str(), &hints, &result);

    if (s != 0) {
        perror("Error getting info for destination");
        if (s == EAI_SYSTEM)
            fprintf(stderr, "looking up www.example.com: %s\n", strerror(errno));
        else
            fprintf(stderr, "looking up www.example.com: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }

    for (rp = result; send_flag && rp != nullptr; rp = rp->ai_next) {

        if (sfd > 0) {
            close(sfd);
        }
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd > 0 && connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1) {
            send_flag = false;
        }
    }
    freeaddrinfo(result);
    if (send_flag) {
        perror("connection failed");
        exit(EXIT_FAILURE);
    }
}

void client::tcp::ClientTCP::send_and_receive(unsigned char* message,
                                              size_t message_len,
                                              unsigned char* received,
                                              ssize_t received_len) {
    received = new unsigned char[BUFFER_SIZE];

    for (int i = 0; i < message_len; i++)
        printf("%x", *(message + i));
    printf("\n");

    send(sock , message , message_len, 0);
    printf("Hello message sent\n");
    received_len = read(sock , received, BUFFER_SIZE);
    if (received_len < -1) {
        perror("error receiving data");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < received_len; i++)
        printf("%x", *(received + i));
    printf("\n");
}

client::fd_type client::tcp::ClientTCP::access_to_socket() const {
    return sock;
}

sockaddr_in client::tcp::ClientTCP::access_to_sockaddr_in() const {
    return serv_addr;
}

void client::tcp::ClientTCP::close_connection() {
    close(sock);
}
