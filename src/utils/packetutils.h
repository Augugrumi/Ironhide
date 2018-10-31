//
// Created by zanna on 08/10/18.
//

#ifndef IRONHIDE_PACKETUTILS_H
#define IRONHIDE_PACKETUTILS_H

#include <string>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <netdb.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/ip.h>
#include <sys/types.h>

namespace utils {

typedef std::pair<struct iphdr, tcphdr> header_ip_tcp;
typedef std::pair<struct iphdr, udphdr> header_ip_udp;

struct pseudo_header {
    uint32_t source_address;
    uint32_t dest_address;
    uint8_t placeholder;
    uint8_t protocol;
    uint16_t udp_length;
};

class PacketUtils {
private:
    PacketUtils() = default;
    const static uint16_t ip_hdr_len;
    const static uint16_t udp_hdr_len;
    const static uint16_t tcp_hdr_len;
    static unsigned short csum(unsigned short*, int);
public:
    static const char* ack;
    static const uint8_t ack_size;
    static std::string int_to_ip(uint32_t ip_int);
    static uint32_t ip_to_int(const char *ip_string);
    static uint16_t retrieve_port(uint16_t port);
    static header_ip_tcp retrieve_ip_tcp_header(unsigned char*);
    static header_ip_udp retrieve_ip_udp_header(unsigned char*);
    static iphdr retrieve_ip_header(unsigned char *);
    static unsigned int udp_packet_header_size(unsigned char*);
    static unsigned int tcp_packet_header_size(unsigned char*);
    static void forge_ip_udp_pkt(unsigned char* data, size_t data_len,
            const char* source_ip, const char* dest_ip,
            uint16_t source_port, uint16_t dest_port,
            struct iphdr*& iph, struct udphdr*& udph,
            unsigned char*& pkt);
    static const char* hostname_to_ip(const char* hostname);
};

#define INT_TO_IP(ip_int) \
    utils::PacketUtils::int_to_ip(ip_int)
#define INT_TO_IP_C_STR(ip_int) \
    utils::PacketUtils::int_to_ip(ip_int).c_str()
#define ACK utils::PacketUtils::ack
#define ACK_SIZE utils::PacketUtils::ack_size
#define IP_UDP_H_LEN(pkt) \
    utils::PacketUtils::udp_packet_header_size((unsigned char*)(pkt))
#define IP_TCP_H_LEN(pkt) \
    utils::PacketUtils::tcp_packet_header_size((unsigned char*)(pkt))
} // namespace utils

#endif //IRONHIDE_PACKETUTILS_H
