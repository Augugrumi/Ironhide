//
// Created by zanna on 05/10/18.
//

#ifndef IRONHIDE_EGRESS_H
#define IRONHIDE_EGRESS_H

#include <string.h>
#include <stdlib.h>
#include <functional>

#include "endpoint.h"
#include "servertcp.h"
#include "serverudp.h"
#include "utils/packetutils.h"
#include "utils/ipheader.h"
#include "utils/sfcheaderstruct.h"
#include "utils/sfcutilities.h"
#include "utils/sfcfixedlengthheader.h"
#include "../client/clientudp.h"
#include "../client/clienttcp.h"

namespace endpoint {

class Egress : public Endpoint {
private:
    void manage_pkt_from_chain(void* args);
    void manage_exiting_tcp_packets(unsigned char* pkt, size_t pkt_len);
    void manage_exiting_udp_packets(unsigned char* pkt, size_t pkt_len);
public:
    void start(uint16_t int_port, uint16_t ext_port) override;
};

} // namespace endpoint

#endif //IRONHIDE_EGRESS_H
