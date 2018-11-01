#ifndef IRONHIDE_CLIENTUDP_H
#define IRONHIDE_CLIENTUDP_H

#include <cstdint>
#include <cstdio>
#include <sys/socket.h>
#include <cstdlib>
#include <netinet/in.h>
#include <cstring>
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
    /**
     * Method to send a message and wait for the response
     * @param message Message to send
     * @param message_len Length of the message
     * @param dst IP of destination
     * @param port Port of the destination
     */
    void send_and_wait_response(unsigned char * message, size_t message_len,
                                const char* dst, uint16_t port);
    /**
     * Method to send a message without waiting the response
     * @param message Message to send
     * @param message_len Length of the message
     * @param dst IP of destination
     * @param port Port of the destination
     */
    fd_type send_only(unsigned char * message, size_t message_len,
                      const char* dst, uint16_t port);
};

} // namespace udp
} // namespace client

#endif //IRONHIDE_CLIENTUDP_H
