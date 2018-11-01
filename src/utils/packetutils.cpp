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
    struct in_addr addr{};
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
    struct iphdr h_ip_tmp{};
    struct iphdr h_ip{};
    struct tcphdr h_tcp_tmp{};
    struct tcphdr h_tcp{};
    memcpy(&h_ip_tmp, pkt, ip_hdr_len);
    auto ip_pkt_len = static_cast<unsigned int>(h_ip_tmp.ihl * 4);
    memcpy(&h_ip, pkt, ip_pkt_len);
    memcpy(&h_tcp_tmp, pkt + ip_pkt_len, tcp_hdr_len);
    auto tcp_pkt_len = static_cast<unsigned int>(h_tcp_tmp.doff * 4);
    memcpy(&h_tcp, pkt + ip_pkt_len, tcp_pkt_len);
    header_ip_tcp res;
    res.first = h_ip;
    res.second = h_tcp;
    return res;
}

utils::header_ip_udp utils::PacketUtils::retrieve_ip_udp_header(
        unsigned char * pkt) {
    struct iphdr h_ip_tmp{};
    struct iphdr h_ip{};
    struct udphdr h_udp{};
    memcpy(&h_ip_tmp, pkt, ip_hdr_len);
    auto ip_pkt_len = static_cast<unsigned int>(h_ip_tmp.ihl * 4);
    memcpy(&h_ip, pkt, ip_pkt_len);
    memcpy(&h_udp, pkt + ip_pkt_len, udp_hdr_len);
    header_ip_udp res;
    res.first = h_ip;
    res.second = h_udp;
    return res;
}

iphdr utils::PacketUtils::retrieve_ip_header(unsigned char * pkt) {
    struct iphdr h_ip_tmp{};
    struct iphdr h_ip{};
    memcpy(&h_ip_tmp, pkt, ip_hdr_len);
    memcpy(&h_ip, pkt, static_cast<size_t>(h_ip_tmp.ihl * 4));
    return h_ip;
}

unsigned int utils::PacketUtils::udp_packet_header_size(unsigned char * pkt) {
    struct iphdr h_ip{};
    memcpy(&h_ip, pkt, ip_hdr_len);
    auto pkt_size = static_cast<unsigned int>(h_ip.ihl * 4);
    return pkt_size + udp_hdr_len;
}

unsigned int utils::PacketUtils::tcp_packet_header_size(unsigned char * pkt) {
    struct iphdr h_ip{};
    memcpy(&h_ip, pkt, ip_hdr_len);
    auto pkt_size = static_cast<unsigned int>(h_ip.ihl * 4);
    struct tcphdr h_tcp{};
    memcpy(&h_tcp, pkt + pkt_size, tcp_hdr_len);
    return pkt_size + h_tcp.doff * 4;
}

/*
    Generic checksum calculation function
*/
unsigned short utils::PacketUtils::csum(unsigned short *ptr,int nbytes) {
    register long sum;
    unsigned short oddbyte;
    register short answer;

    sum=0;
    while(nbytes>1) {
        sum+=*ptr++;
        nbytes-=2;
    }
    if(nbytes==1) {
        oddbyte=0;
        *((u_char*)&oddbyte)=*(u_char*)ptr;
        sum+=oddbyte;
    }

    sum = (sum>>16)+(sum & 0xffff);
    sum = sum + (sum>>16);
    answer=(short)~sum;

    return static_cast<unsigned short>(answer);
}

void utils::PacketUtils::forge_ip_udp_pkt(unsigned char *data, size_t data_len,
                                          const char * source_ip,
                                          const char * dest_ip,
                                          uint16_t source_port,
                                          uint16_t dest_port,
                                          struct iphdr *&iph,
                                          struct udphdr *&udph,
                                          unsigned char *&datagram) {
    char* pseudogram;

    //zero out the packet buffer
    memset(datagram, 0, 4096);

    //IP header
    iph = (struct iphdr *) datagram;

    //UDP header
    udph = (struct udphdr *) (datagram + sizeof (struct iphdr));

    struct pseudo_header psh{};

    //Data part
    unsigned char* data_ptr = datagram + sizeof(struct iphdr) + sizeof(struct udphdr);
    std::strncpy((char *)data_ptr, (char *)data, data_len);

    //Fill in the IP Header
    iph->ihl = 5;
    iph->version = 4;
    iph->tos = 0;
    iph->tot_len = sizeof (struct iphdr) + sizeof (struct udphdr) + data_len;
    iph->id = static_cast<__be16>(htonl (54321)); //Id of this packet
    iph->frag_off = 0;
    iph->ttl = 255;
    iph->protocol = IPPROTO_UDP;
    iph->check = 0;      //Set to 0 before calculating checksum
    iph->saddr = inet_addr(source_ip);    //Spoof the source ip address
    iph->daddr = inet_addr(dest_ip);

    //Ip checksum
    iph->check = csum ((unsigned short *) datagram, iph->tot_len);

    //UDP header
    udph->source = htons(source_port);//(source_port);
    udph->dest = htons(dest_port);
    udph->len = htons(static_cast<uint16_t>(8 + data_len)); //udp header size
    udph->check = 0; //leave checksum 0 now, filled later by pseudo header

    //Now the UDP checksum using the pseudo header
    psh.source_address = inet_addr( source_ip );
    psh.dest_address = inet_addr(dest_ip);
    psh.placeholder = 0;
    psh.protocol = IPPROTO_UDP;
    psh.udp_length = htons(sizeof(struct udphdr) + data_len);

    int psize = sizeof(struct pseudo_header) + sizeof(struct udphdr) + data_len;
    pseudogram = (char*)malloc(static_cast<size_t>(psize));

    memcpy(pseudogram , (char*) &psh , sizeof (struct pseudo_header));
    memcpy(pseudogram + sizeof(struct pseudo_header) , udph ,
            sizeof(struct udphdr) + data_len);

    udph->check = csum( (unsigned short*) pseudogram , psize);

}