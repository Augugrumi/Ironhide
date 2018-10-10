//
// Created by zanna on 05/10/18.
//

#ifndef IRONHIDE_CLIENTTCP_H
#define IRONHIDE_CLIENTTCP_H

#include <cstdint>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "client.h"

namespace client {
namespace tcp {

class ClientTCP {
private:
    sockaddr_in address;
    sockaddr_in serv_addr;
    fd_type sock;

public:
    void connect_to_server(char* dst, uint16_t port);
    void send_and_receive(unsigned char* message, size_t message_len,
                          unsigned char* received, ssize_t received_len);
    fd_type access_to_socket() const;
    void close_connection();
};

} // namespace tcp
} // namespace client

#endif //IRONHIDE_CLIENTTCP_H
