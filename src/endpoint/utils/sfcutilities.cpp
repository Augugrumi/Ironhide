//
// Created by zanna on 28/09/18.
//

#include <log.h>
#include "sfcutilities.h"

namespace utils {
namespace sfc_header {

const uint8_t SFCUtilities::HEADER_SIZE = 24;

uint32_t SFCUtilities::ip_to_int(const char *ip_string) {
    return inet_addr(ip_string);
}

std::string SFCUtilities::int_to_ip(uint32_t ip_int) {
    struct in_addr addr;
    addr.s_addr = ip_int;
    char* buf = inet_ntoa(addr);// conversion BigEndian/LittleEndian?
    std::string ip_string(buf);
    return ip_string;
}

struct sfc_header SFCUtilities::create_header(
        uint32_t service_path_id, uint32_t service_index,
        char *source_address, uint16_t source_port,
        char *dest_address, uint16_t dest_port,
        uint16_t ttl, uint8_t direction) {
    struct sfc_header header;
    header.ver = 0;
    header.oam = 0;
    header.u1 = 0;
    header.ttl = ttl;
    header.u2 = 0;
    header.md_type = 0x1; // oxF otherwise
    header.prot = 0x1;
    header.p_id = service_path_id;
    header.si = service_index;
    header.source_address = ip_to_int(source_address);
    header.source_port = source_port;
    header.destination_address = ip_to_int(dest_address);
    header.destination_port = dest_port;
    header.direction_flag = direction;

    return header;
}

void SFCUtilities::prepend_header(unsigned char*& no_header_pkt, size_t pkt_size,
          struct sfc_header header, unsigned char*& pkt_w_header) {
    pkt_w_header = new unsigned char[HEADER_SIZE + pkt_size];
    memcpy((char*)pkt_w_header, &header, HEADER_SIZE);
    memcpy((char*)pkt_w_header + HEADER_SIZE, no_header_pkt, pkt_size);
}

struct sfc_header SFCUtilities::retrieve_header(uint8_t* packet) {
    struct sfc_header header;
    memcpy(&header, packet, sizeof(header));
    return header;
}

void SFCUtilities::retrieve_payload(uint8_t* packet, size_t size, uint8_t*& payload) {
    payload = new uint8_t[size - HEADER_SIZE]; // 24 is the size of the header
    memcpy(payload, packet + HEADER_SIZE, size - HEADER_SIZE);
}

} // namespace sfcheader
} // namespace utils