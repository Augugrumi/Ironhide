//
// Created by zanna on 04/10/18.
//

#ifndef IRONHIDE_SERVERUDP_H
#define IRONHIDE_SERVERUDP_H

#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <atomic>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>

#include "utils/asynctaskexecutor.h"
#include "server.h"

namespace server {
namespace udp {

typedef struct {
    int socket_fd;
    struct sockaddr_in client_address;
    char* pkt;
    ssize_t pkt_len;
} udp_pkt_mngmnt_args;

class ServerUDP : public Server {
public:
    ServerUDP(uint16_t port);
    void run() override;
};

} // namespace tcp
} // namespace server

#endif //IRONHIDE_SERVERUDP_H
