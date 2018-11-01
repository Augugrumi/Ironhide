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
#include <netdb.h>
#include <string>

#include "client.h"

namespace client {
namespace tcp {

class ClientTCP {
private:
    sockaddr_in address;
    sockaddr_in serv_addr;
    fd_type sock;

public:
    /**
     * To connect to a remote server
     * @param dst IP of server
     * @param port Port used by the server
     */
    void connect_to_server(const char* dst, uint16_t port);
    /**
     * Method to send a message and wait for the response
     * @param message Message to send
     * @param message_len Length of the message
     * @param dst IP of destination
     * @param port Port of the destination
     */
    void send_and_receive(unsigned char* message, size_t message_len,
                          unsigned char* received, ssize_t received_len);
    /**
     * To access the socket used to send data
     * @return the id of the socket file descriptor
     */
    fd_type access_to_socket() const;
    /**
     * To access data of the connection
     * @return sockaddr_in
     */
    sockaddr_in access_to_sockaddr_in() const;
    /**
     * To gracefully close a connection
     */
    void close_connection();
};

} // namespace tcp
} // namespace client

#endif //IRONHIDE_CLIENTTCP_H
