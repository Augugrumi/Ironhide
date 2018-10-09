//
// Created by zanna on 05/10/18.
//

#include "endpoint.h"

classifier::Classifier endpoint::Endpoint::classifier_;

int endpoint::Endpoint::retrieve_file_descriptior(char *source_ip,
                                                  uint16_t source_port,
                                                  char *dest_ip,
                                                  uint16_t dest_port,
                                                  endpoint::Protocol pr) {
    // TODO call to the remote db

    return 0;
}

void endpoint::Endpoint::add_entry(endpoint::ConnectionEntry ce,
                                   endpoint::socket_fd fd,
                                   endpoint::Protocol protocol) {
    connection_map[ce] = fd;
    // TODO call to remote db
}

void endpoint::Endpoint::delete_entry(endpoint::ConnectionEntry ce) {
    auto it = connection_map.find(ce);
    if (it != connection_map.end())
        connection_map.erase(ce);
}

endpoint::socket_fd endpoint::Endpoint::retrieve_connection(
        endpoint::ConnectionEntry ce) {
    auto it = connection_map.find(ce);
    if (it == connection_map.end())
        return -1;
    return it->second;
}

