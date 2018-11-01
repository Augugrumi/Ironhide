//
// Created by zanna on 28/09/18.
//

#ifndef IRONHIDE_SFC_UTILITIES_H
#define IRONHIDE_SFC_UTILITIES_H

#include <stdint.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>

#include "packetutils.h"
#include "sfcheaderstruct.h"

namespace utils {
namespace sfc_header {

/**
 * Utility class to manipulate SFC header
 */
class SFCUtilities {
public:
    /**
     * SFC header size
     */
    static const uint8_t HEADER_SIZE;

    /**
     * Create sfc header setting data
     * @param service_path_id ID of the chain
     * @param service_index Index of the SF
     * @param source_address IP of the source
     * @param source_port Port used by the source
     * @param dest_address IP of the destination
     * @param dest_port Port used by the destination
     * @param ttl Time to live
     * @param direction 0 -> ingress to egress, 1 -> egress to ingress
     * @return a new sfc header
     */
    static struct sfc_header create_header(
            uint32_t service_path_id, uint32_t service_index,
            const char *source_address, uint16_t source_port,
            const char *dest_address, uint16_t dest_port,
            uint16_t ttl = 64, uint8_t direction = 0
    );

    /**
     * Prepend the sfc header to a packet.
     * The header and the packet without the header are copied to contiguous
     * memory locations to be used as a unoque packet
     * @param no_header_pkt Packet to which prepend the header
     * @param pkt_size Size of the packet without SFC header
     * @param header SFC header to prepend
     * @param pkt_w_header Pointer to the packet with the header
     */
    static void prepend_header(
            unsigned char* no_header_pkt, size_t pkt_size,
            struct sfc_header header, unsigned char*& pkt_w_header);

    /**
     * Retrieve the SFC header from a packet.
     * It copies a memory portion as big as an sfc header to a new sfc
     * header struct to access data
     * @param packet Packet that has as suffix the sfc header
     * @return the header created
     */
    static struct sfc_header retrieve_header(uint8_t* packet);

    /**
     * Retrive the payload
     * @param packet Packet with the SFC header
     * @param size Size of the packet
     * @param payload Payload data retrieved
     */
    static void retrieve_payload(uint8_t* packet, size_t size,
                                 uint8_t*& payload);
};

#define SFC_HDR utils::sfc_header::SFCUtilities::HEADER_SIZE

} // namespace sfcheader
} // namespace utils

#endif //IRONHIDE_SFC_UTILITIES_H
