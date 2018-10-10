//
// Created by zanna on 05/10/18.
//

#ifndef IRONHIDE_ENDPOINT_H
#define IRONHIDE_ENDPOINT_H

#include <cstdint>
#include <map>

#include "connectionentry.h"
#include "../classifier/classifier.h"
#include "../db/dbquery.h"

namespace endpoint {

enum Protocol{TCP, UDP};

typedef int socket_fd;

class Endpoint {
private:
    std::map<ConnectionEntry, socket_fd> connection_map;
protected:
    static classifier::Classifier classifier_;
    // TODO inizialize
    static db::DBQuery roulette_;
    // TODO to implement
    void add_entry(ConnectionEntry, socket_fd, Protocol);
    void delete_entry(ConnectionEntry); // even by socket?
    socket_fd retrieve_connection(ConnectionEntry);
    int retrieve_file_descriptor(char* source_ip, uint16_t source_port,
                                 char* dest_ip, uint16_t dest_port,
                                 Protocol pr);
public:
    virtual void start(uint16_t int_port, uint16_t ext_port) = 0;
};

} // namespace endpoint

#endif //IRONHIDE_ENDPOINT_H
