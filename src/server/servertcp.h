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
#include <sys/poll.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <unistd.h>
#include <cstring>
#include <sys/ioctl.h>
#include <memory>

#include "utils/asynctaskexecutor.h"
#include "server.h"
#include "log.h"
#include "connectionentry.h"

namespace server {
namespace tcp {

/**
* Structure that contains data used for managing a packet received
*/
typedef struct {
    int new_socket_fd;
    struct sockaddr_in client_address;
    bool first_run;
    endpoint::ConnectionEntry* ce;
    char* pkt;
    size_t pkt_size;
} tcp_pkt_mngmnt_args;

typedef std::shared_ptr<tcp_pkt_mngmnt_args> args_ptr;

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
