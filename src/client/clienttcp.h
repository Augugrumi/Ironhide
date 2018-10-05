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
public:
    void connect_and_send(unsigned char* message, size_t message_len,
                          char* dst, uint16_t port);
};

} // namespace tcp
} // namespace client

#endif //IRONHIDE_CLIENTTCP_H
