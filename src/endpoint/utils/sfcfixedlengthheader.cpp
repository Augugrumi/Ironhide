#include "sfcfixedlengthheader.h"

utils::sfc_header::SFCFixedLengthHeader::SFCFixedLengthHeader(
        const unsigned char *byte_array) noexcept {
    memcpy(&header, byte_array, sizeof(header));
}

utils::sfc_header::SFCFixedLengthHeader::SFCFixedLengthHeader(
        std::shared_ptr<unsigned char> byte_array) noexcept {
    memcpy(&header, byte_array.get(), sizeof(header));
}

uint32_t utils::sfc_header::SFCFixedLengthHeader::get_service_path_id() const {
    return header.p_id;
}

uint32_t utils::sfc_header::SFCFixedLengthHeader::get_service_index() const {
    return header.si;
}

std::string
utils::sfc_header::SFCFixedLengthHeader::get_source_ip_address() const {
    return PacketUtils::int_to_ip(header.source_address);
}

std::string
utils::sfc_header::SFCFixedLengthHeader::get_destination_ip_address() const {
    return PacketUtils::int_to_ip(header.destination_address);
}

uint16_t utils::sfc_header::SFCFixedLengthHeader::get_source_port() const {
    return header.source_port;
}

uint16_t utils::sfc_header::SFCFixedLengthHeader::get_destination_port() const {
    return header.destination_port;
}

uint8_t utils::sfc_header::SFCFixedLengthHeader::get_direction_flag() const {
    return header.direction_flag;
}

uint16_t utils::sfc_header::SFCFixedLengthHeader::get_ttl() const {
    return header.ttl;
}

void utils::sfc_header::SFCFixedLengthHeader::set_service_index(
        uint32_t service_index) {
    header.si = service_index;
}

void utils::sfc_header::SFCFixedLengthHeader::set_ttl(uint16_t ttl) {
    header.ttl = ttl;
}

struct sfc_header utils::sfc_header::SFCFixedLengthHeader::get_header() const {
    return header;
}