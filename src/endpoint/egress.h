//
// Created by zanna on 05/10/18.
//

#ifndef IRONHIDE_EGRESS_H
#define IRONHIDE_EGRESS_H

#include "endpoint.h"
#include "servertcp.h"
#include "serverudp.h"

namespace endpoint {

class Egress : public Endpoint {
private:
    static void manage_pkt_from_chain(void* args);
    static void manage_exiting_tcp_packets(void*args);
    static void manage_exiting_udp_packets(void* args);
public:
    void start(uint16_t int_port, uint16_t ext_port) override;
};

} // namespace endpoint

#endif //IRONHIDE_EGRESS_H
