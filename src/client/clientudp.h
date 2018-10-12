//
// Created by zanna on 05/10/18.
//

#ifndef IRONHIDE_CLIENTUDP_H
#define IRONHIDE_CLIENTUDP_H

#include <cstdint>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <string>

#include "client.h"
#include "log.h"

namespace client{
namespace udp{

class ClientUDP {
public:
    void send_and_wait_response(unsigned char * message, size_t message_len,
                                char* dst, uint16_t port);
    fd_type send_only(unsigned char * message, size_t message_len,
                      char* dst, uint16_t port);
};

} // namespace udp
} // namespace client

#endif //IRONHIDE_CLIENTUDP_H
