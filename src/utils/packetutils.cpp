//
// Created by zanna on 08/10/18.
//
#include "packetutils.h"

const uint16_t utils::PacketUtils::ip_hdr_len = sizeof(iphdr);
const uint16_t utils::PacketUtils::udp_hdr_len = sizeof(udphdr);
const uint16_t utils::PacketUtils::tcp_hdr_len = sizeof(tcphdr);
const char* utils::PacketUtils::ack("ACK");
const uint8_t utils::PacketUtils::ack_size(3);

std::string utils::PacketUtils::int_to_ip(uint32_t ip_int) {
    struct in_addr addr;
    addr.s_addr = ip_int;
    char* buf = inet_ntoa(addr);
    std::string ip_string(buf);
    return ip_string;
}

uint32_t utils::PacketUtils::ip_to_int(const char *ip_string) {
    return inet_addr(ip_string);
}

uint16_t utils::PacketUtils::retrieve_port(uint16_t port) {
    return htons(port);
}

utils::header_ip_tcp utils::PacketUtils::retrieve_ip_tcp_header(
        unsigned char * pkt) {
    struct iphdr h_ip_tmp;
    struct iphdr h_ip;
    struct tcphdr h_tcp_tmp;
    struct tcphdr h_tcp;
    memcpy(&h_ip_tmp, pkt, ip_hdr_len);
    unsigned int ip_pkt_len = h_ip_tmp.ihl * 4;
    memcpy(&h_ip, pkt, ip_pkt_len);
    memcpy(&h_tcp_tmp, pkt + ip_pkt_len, tcp_hdr_len);
    unsigned int tcp_pkt_len = h_tcp_tmp.doff * 4;
    memcpy(&h_tcp, pkt + ip_pkt_len, tcp_pkt_len);
    header_ip_tcp res;
    res.first = h_ip;
    res.second = h_tcp;
    return res;
}

utils::header_ip_udp utils::PacketUtils::retrieve_ip_udp_header(
        unsigned char * pkt) {
    struct iphdr h_ip_tmp;
    struct iphdr h_ip;
    struct udphdr h_udp;
    memcpy(&h_ip_tmp, pkt, ip_hdr_len);
    unsigned int ip_pkt_len = h_ip_tmp.ihl * 4;
    memcpy(&h_ip, pkt, ip_pkt_len);
    memcpy(&h_udp, pkt + ip_pkt_len, udp_hdr_len);
    header_ip_udp res;
    res.first = h_ip;
    res.second = h_udp;
    return res;
}

iphdr utils::PacketUtils::retrieve_ip_header(unsigned char * pkt) {
    struct iphdr h_ip_tmp;
    struct iphdr h_ip;
    memcpy(&h_ip_tmp, pkt, ip_hdr_len);
    memcpy(&h_ip, pkt, h_ip_tmp.ihl * 4);
    return h_ip;
}

unsigned int utils::PacketUtils::udp_packet_header_size(unsigned char * pkt) {
    struct iphdr h_ip;
    memcpy(&h_ip, pkt, ip_hdr_len);
    unsigned int pkt_size = h_ip.ihl * 4;
    return pkt_size + udp_hdr_len;
}

unsigned int utils::PacketUtils::tcp_packet_header_size(unsigned char * pkt) {
    struct iphdr h_ip;
    memcpy(&h_ip, pkt, ip_hdr_len);
    unsigned int pkt_size = h_ip.ihl * 4;
    struct tcphdr h_tcp;
    memcpy(&h_tcp, pkt + pkt_size, tcp_hdr_len);
    return pkt_size + h_tcp.doff * 4;
}
