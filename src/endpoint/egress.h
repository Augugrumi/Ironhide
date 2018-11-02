#ifndef IRONHIDE_EGRESS_H
#define IRONHIDE_EGRESS_H

#include "config.h"

#include <cstring>
#include <cstdlib>
#include <linux/ip.h>
#include <functional>
#include <fcntl.h>
#include <fstream>
#include <vector>
#include <iterator>
#include <ostream>
#include <sys/socket.h>

#include "endpoint.h"
#include "servertcp.h"
#include "serverudp.h"
#include "packetutils.h"
#include "utils/sfcheaderstruct.h"
#include "utils/sfcutilities.h"
#include "utils/sfcfixedlengthheader.h"
#include "../client/clientudp.h"
#include "../client/clienttcp.h"

namespace endpoint {

class Egress : public Endpoint {
private:
    /**
     * It manages incoming UDP packets that it has received from the chain
     * Once a packet is received the packet is read (SFC header and IP header)
     * in order to manage it differently depending on the protocol used.
     * @param args udp_pkt_mngmnt_args struct that contains data to manage
     * packets from the chain.
     */
    void manage_pkt_from_chain(void* args);
    /**
     * The method handles a TCP connection with a defined destination.
     * First the method checks if there is an existing connection (if not it
     * creates one), then sends the packet to the destination waiting for the
     * response.
     * @param pkt Packet to send
     * @param pkt_len Lenght of the packet to send
     * @param ce Information on the connection
     * @param fd Socket used for the connection
     */
    void manage_exiting_tcp_packets(unsigned char* pkt, size_t pkt_len,
                                    const ConnectionEntry& ce, socket_fd fd);
    /**
     * Method used to manage TCP connection with the destination.
     * It creates a connection and it sends data using raw sockets, than it
     * waits for the response.
     * @param pkt Packet to send
     * @param pkt_len Lenght of the packet to send
     * @param ce Information on the connection
     * @param fd Socket used for the connection
     */
    void manage_exiting_udp_packets(unsigned char* pkt, size_t pkt_len,
                                    const ConnectionEntry& ce, socket_fd fd);
public:
    /**
     * Constructor
     * @param ext_port Port exposed to the external
     * @param int_port Port exposed to the chain
     */
    Egress(uint16_t ext_port, uint16_t int_port);
    /**
     * Start the egress
     */
    void start() override;
};

} // namespace endpoint

#endif //IRONHIDE_EGRESS_H
