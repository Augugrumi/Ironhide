//
// Created by zanna on 05/10/18.
//

#ifndef IRONHIDE_ENDPOINT_H
#define IRONHIDE_ENDPOINT_H

#include <cstdint>

namespace endpoint {

enum Protocol{TCP, UDP};


class Endpoint {
protected:
    int retrieve_file_descriptior(char* source_ip, uint16_t source_port,
                                  char* dest_ip, uint16_t dest_port,
                                  Protocol pr);
public:
    virtual void start(uint16_t int_port, uint16_t ext_port) = 0;
};

} // namespace endpoint

#endif //IRONHIDE_ENDPOINT_H
