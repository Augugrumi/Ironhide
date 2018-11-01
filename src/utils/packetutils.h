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

/**
 * Class to manipulate TCP/UDP IP headers and packets
 */
class PacketUtils {
private:
    PacketUtils() = default;
    const static uint16_t ip_hdr_len;
    const static uint16_t udp_hdr_len;
    const static uint16_t tcp_hdr_len;
    static unsigned short csum(unsigned short*, int);
public:
    /**
     * Default ACK string packet
     */
    static const char* ack;
    /**
     * Default ACk packet length
     */
    static const uint8_t ack_size;
    /**
     * To convert an IP from int notation to dotted notation
     * @param ip_int int that represent an IP
     * @return String that represent IP in dotted notation
     */
    static std::string int_to_ip(uint32_t ip_int);
    /**
     * To convert an IP from dotted notation to int notation
     * @param ip_string char* that represent an IP
     * @return int that represent the IP
     */
    static uint32_t ip_to_int(const char *ip_string);
    /**
     * Performs htons
     */
    static uint16_t retrieve_port(uint16_t port);
    /**
     * Retrieve the ip tcp header
     * @return pair in which the first is the ip header and the second is the
     * tcp header
     */
    static header_ip_tcp retrieve_ip_tcp_header(unsigned char*);
    /**
     * Retrieve the ip udp header
     * @return pair in which the first is the ip header and the second is the
     * udp header
     */
    static header_ip_udp retrieve_ip_udp_header(unsigned char*);
    /**
     * Retrieve the ip header
     * @return a struct that represent the ip header
     */
    static iphdr retrieve_ip_header(unsigned char *);
    /**
     * calculate the size of the udp packet
     * @return size of the udp packet
     */
    static unsigned int udp_packet_header_size(unsigned char*);
    /**
     * calculate the size of the tcp packet
     * @return size of the tcp packet
     */
    static unsigned int tcp_packet_header_size(unsigned char*);
    /**
     * Create a udp packet, used by raw socket
     * @param data the payload
     * @param data_len length of the payload
     * @param source_ip
     * @param dest_ip
     * @param source_port
     * @param dest_port
     * @param iph ip header struct
     * @param udph udp header struct
     * @param pkt final packet forged
     */
    static void forge_ip_udp_pkt(unsigned char* data, size_t data_len,
            const char* source_ip, const char* dest_ip,
            uint16_t source_port, uint16_t dest_port,
            struct iphdr*& iph, struct udphdr*& udph,
            unsigned char*& pkt);
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
