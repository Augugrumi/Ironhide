//
// Created by zanna on 08/10/18.
//
#include "packetutils.h"

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
    struct iphdr h_ip;
    struct tcphdr h_tcp;
    memcpy(&h_ip, pkt, sizeof(iphdr));
    memcpy(&h_tcp, pkt + sizeof(iphdr), sizeof(tcphdr));
    header_ip_tcp res;
    res.first = h_ip;
    res.second = h_tcp;
    return res;
}

utils::header_ip_udp utils::PacketUtils::retrieve_ip_udp_header(
        unsigned char * pkt) {
    struct iphdr h_ip;
    struct udphdr h_udp;
    memcpy(&h_ip, pkt, sizeof(iphdr));
    memcpy(&h_udp, pkt + sizeof(iphdr), sizeof(udphdr));
    header_ip_udp res;
    res.first = h_ip;
    res.second = h_udp;
    return res;
}

iphdr utils::PacketUtils::retrieve_ip_header(unsigned char * pkt) {
    struct iphdr h_ip;
    memcpy(&h_ip, pkt, sizeof(iphdr));
    return h_ip;
}
