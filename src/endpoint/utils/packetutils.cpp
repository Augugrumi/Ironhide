//
// Created by zanna on 08/10/18.
//

#include "packetutils.h"

std::string utils::PacketUtils::int_to_ip(uint32_t ip_int) {
    struct in_addr addr;
    addr.s_addr = ip_int;
    char* buf = inet_ntoa(addr);
    std::string ip_string(buf);
    return ip_string;
}

uint16_t utils::PacketUtils::retrieve_port(uint16_t port) {
    return htons(port);
}

utils::header_ip_tcp utils::PacketUtils::retrieve_ip_tcp_header(
        unsigned char * pkt) {
    struct ip_header_t h_ip;
    struct tcphdr h_tcp;
    memcpy(&h_ip, pkt, sizeof(ip_header_t));
    memcpy(&h_tcp, pkt + sizeof(ip_header_t), sizeof(tcphdr));
    header_ip_tcp res;
    res.first = h_ip;
    res.second = h_tcp;
    return res;
}

utils::header_ip_udp utils::PacketUtils::retrieve_ip_udp_header(
        unsigned char * pkt) {
    struct ip_header_t h_ip;
    struct udphdr h_udp;
    memcpy(&h_ip, pkt, sizeof(ip_header_t));
    memcpy(&h_udp, pkt + sizeof(ip_header_t), sizeof(udphdr));
    header_ip_udp res;
    res.first = h_ip;
    res.second = h_udp;
    return res;
}

utils::ip_header_t utils::PacketUtils::retrieve_ip_header(unsigned char * pkt) {
    struct ip_header_t h_ip;
    memcpy(&h_ip, pkt, sizeof(ip_header_t));
    return h_ip;
}
