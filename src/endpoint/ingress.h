//
// Created by zanna on 05/10/18.
//

#ifndef IRONHIDE_INGRESS_H
#define IRONHIDE_INGRESS_H

#include "endpoint.h"
#include "servertcp.h"
#include "serverudp.h"

namespace endpoint {

// TODO - save all connection on a map (socket fd)
//      - once a pkt arrives -> classificator -> retrieve the sfid index -> next
//      - once a pkt comes back -> db to check the fd(?) -> send to open connection
class Ingress : public Endpoint {
private:
    static void manage_entering_tcp_packets(void* args);
    static void manage_entering_udp_packets(void* args);
    static void manage_pkt_from_chain(void* args);
public:
    void start(uint16_t int_port, uint16_t ext_port) override;
};

} // namespace endpoint

#endif //IRONHIDE_INGRESS_H
