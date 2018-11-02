#ifndef IRONHIDE_INGRESS_H
#define IRONHIDE_INGRESS_H

#include "config.h"

#include <cstring>
#include <cstdlib>
#include <linux/ip.h>
#include <cstdint>
#include <arpa/inet.h>
#include <string>
#include <sys/fcntl.h>

#include "endpoint.h"
#include "servertcp.h"
#include "serverudp.h"
#include "packetutils.h"
#include "utils/sfcheaderstruct.h"
#include "utils/sfcutilities.h"
#include "utils/sfcfixedlengthheader.h"
#include "../client/clientudp.h"

namespace endpoint {

class Ingress : public Endpoint {
private:
    /**
     * Method that manages incoming entering TCP connections.
     * Once a packet is received it will be classified. After roulette will be
     * updated to maintain data of the connection. To the packet is added the
     * SFC header and it is sent to the chain.
     * When the client close the connection it cleanup roulette db.
     * @param args tcp_pkt_mngmnt_args struct that contains data to manage
     * entering packets.
     */
    void manage_entering_tcp_packets(void* args);
    /**
     * Method used to manage entering UDP connections.
     * Once a packet is received it will be classified. After roulette will be
     * updated to maintain data of the connection. To the packet is added the
     * SFC header and it is sent to the chain.
     * @param args udp_pkt_mngmnt_args struct that contains data to manage
     * entering packets.
     */
    void manage_entering_udp_packets(void* args);
    /**
     * Method used to manage UDP packets received from the chain.
     * Once a packet is received the packet is read (SFC header and IP header)
     * in order to manage it differently depending on the protocol used.
     * @param args udp_pkt_mngmnt_args struct that contains data to manage
     * packets from the chain.
     */
    void manage_pkt_from_chain(void* args);
public:
    /**
     * Constructor
     * @param ext_port Port exposed to the external
     * @param int_port Port exposed to the chain
     */
    Ingress(uint16_t ext_port, uint16_t int_port);
    /**
     * Start the ingress
     */
    void start() override;
};

} // namespace endpoint

#endif //IRONHIDE_INGRESS_H
