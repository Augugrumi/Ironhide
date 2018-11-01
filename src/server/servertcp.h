//
// Created by zanna on 04/10/18.
//

#ifndef IRONHIDE_SERVERTCP_H
#define IRONHIDE_SERVERTCP_H

#include <netinet/in.h>
#include <pthread.h>
#include <csignal>
#include <cstdio>
#include <cstdlib>
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

/**
* Structure that contains data used for managing a packet received
*/
typedef struct {
    int new_socket_fd;
    struct sockaddr_in client_address;
} tcp_pkt_mngmnt_args;

class ServerTCP : public Server {
public:
    /**
     * Constructor
     * @param port Port on which the udp server waits for packets
     */
    ServerTCP(uint16_t port);
    /**
     * Start server
     */
    void run() override;
};

} // namespace tcp
} // namespace server

#endif //IRONHIDE_SERVERTCP_H
