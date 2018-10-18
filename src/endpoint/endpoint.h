//
// Created by zanna on 05/10/18.
//

#ifndef IRONHIDE_ENDPOINT_H
#define IRONHIDE_ENDPOINT_H

#include <cstdint>
#include <stdio.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <map>

#include "connectionentry.h"
#include "../classifier/classifier.h"
#include "../db/dbquery.h"

namespace endpoint {

typedef int socket_fd;

class Endpoint {
private:
    std::map<ConnectionEntry, socket_fd> connection_map;
    std::map<ConnectionEntry, std::pair<socket_fd, sockaddr_in>> connection_map_2;
    std::map<ConnectionEntry, std::string> map_to_remote;
    std::string my_ip_;
    uint16_t ext_port_;
    uint16_t int_port_;
protected:
    static classifier::Classifier classifier_;
    // TODO inizialize
    static db::DBQuery* roulette_;
    // TODO to implement
    void add_entry(ConnectionEntry, socket_fd, db::endpoint_type, db::protocol_type);
    void add_entry(ConnectionEntry, socket_fd, sockaddr_in, db::endpoint_type, db::protocol_type);
    void update_entry(ConnectionEntry, endpoint::socket_fd, db::endpoint_type);
    void delete_entry(ConnectionEntry); // even by socket?
    socket_fd retrieve_connection(ConnectionEntry);
    std::pair<socket_fd, sockaddr_in> retrieve_connection_2(ConnectionEntry);

    uint16_t get_internal_port() const;
    uint16_t get_external_port() const;

    // TODO set it somewhere -> passing as argument to main so it easier to dockerize?
    void set_my_ip(const std::string& my_ip);
    std::string get_my_ip() const;
    void retrieve_ip();
public:
    Endpoint(uint16_t ext_port, uint16_t int_port);
    virtual void start() = 0;
    // TODO to refactor
    static void set_remote(const char* ip, uint16_t port);
};

} // namespace endpoint

#endif //IRONHIDE_ENDPOINT_H
