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

class SFCUtilities {
public:
    static const uint8_t HEADER_SIZE;

    static struct sfc_header create_header(
            uint32_t service_path_id, uint32_t service_index,
            const char *source_address, uint16_t source_port,
            const char *dest_address, uint16_t dest_port,
            uint16_t ttl = 64, uint8_t direction = 0
    );

    static void prepend_header(
            unsigned char* no_header_pkt, size_t pkt_size,
            struct sfc_header header, unsigned char*& pkt_w_header);

    static struct sfc_header retrieve_header(uint8_t* packet);
    static void retrieve_payload(uint8_t* packet, size_t size,
                                 uint8_t*& payload);
};

#define SFC_HDR utils::sfc_header::SFCUtilities::HEADER_SIZE

} // namespace sfcheader
} // namespace utils

#endif //IRONHIDE_SFC_UTILITIES_H
