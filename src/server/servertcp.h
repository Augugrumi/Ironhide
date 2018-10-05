//
// Created by zanna on 04/10/18.
//

#ifndef IRONHIDE_SERVERTCP_H
#define IRONHIDE_SERVERTCP_H

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
namespace tcp {

typedef struct tcp_pkt_mngmnt_args {
    int new_socket_fd;
    struct sockaddr_in client_address;
} tcp_pkt_mngmnt_args;

class ServerTCP : public Server {
public:
    ServerTCP(uint16_t port);
    void run() override;
};

} // namespace tcp
} // namespace server

#endif //IRONHIDE_SERVERTCP_H
