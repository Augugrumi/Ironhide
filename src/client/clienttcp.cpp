//
// Created by zanna on 05/10/18.
//

#include "clienttcp.h"

void client::tcp::ClientTCP::connect_and_send(unsigned char *message,
                                              size_t message_len,
                                              char *dst,
                                              uint16_t port) {
    struct sockaddr_in address;
    fd_type sock = 0;
    ssize_t valread;
    struct sockaddr_in serv_addr;

    char buffer[BUFFER_SIZE] = {0};
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket creation error");
        exit(EXIT_FAILURE);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, dst, &serv_addr.sin_addr)<=0) {
        perror("invalid address / address not supported");
        exit(EXIT_FAILURE);
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connection failed");
        exit(EXIT_FAILURE);
    }
    send(sock , message , message_len, 0);
    printf("Hello message sent\n");
    valread = read( sock , buffer, BUFFER_SIZE);
    if (valread < -1) {
        perror("error receiving data");
        exit(EXIT_FAILURE);
    }

    close(sock);
}
