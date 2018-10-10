//
// Created by zanna on 05/10/18.
//

#ifndef IRONHIDE_INGRESS_H
#define IRONHIDE_INGRESS_H

#include <string.h>
#include <stdlib.h>
#include <linux/ip.h>

#include "endpoint.h"
#include "servertcp.h"
#include "serverudp.h"
#include "packetutils.h"
#include "utils/sfcheaderstruct.h"
#include "utils/sfcutilities.h"
#include "utils/sfcfixedlengthheader.h"
#include "../client/clientudp.h"

namespace endpoint {

// TODO - save all connection on a map (socket fd)
//      - once a pkt arrives -> classificator -> retrieve the sfid index -> next
//      - once a pkt comes back -> db to check the fd(?) -> send to open connection
class Ingress : public Endpoint {
private:
    void manage_entering_tcp_packets(void* args);
    void manage_entering_udp_packets(void* args);
    void manage_pkt_from_chain(void* args);
public:
    void start(uint16_t int_port, uint16_t ext_port) override;
};

} // namespace endpoint

#endif //IRONHIDE_INGRESS_H
