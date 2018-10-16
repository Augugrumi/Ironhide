//
// Created by zanna on 05/10/18.
//

#include "classifier.h"

char* classifier::Classifier::classify_pkt(unsigned char *pkt,
                                                  size_t pkt_len) {
    classifier::Classifier::Mapper m;
    m.set_pkt(pkt, pkt_len);
    // TODO retrieve from harbor?
    sfc_map[classifier::Classifier::Mapper::defaultUDPMapper()] = "0";
    sfc_map[classifier::Classifier::Mapper::defaultTCPMapper()] = "0";
    return sfc_map.at(m);
}

void classifier::Classifier::Mapper::set_pkt(unsigned char *pkt,
                                             size_t pkt_len) {
    //TODO take the packet, check the header and at least classify based on
    //     protocol. At the end set the type_ to not save all the pkt
    //     do not know how to do? default chain
    if (pkt_len > sizeof(iphdr)) {
        iphdr h_ip = utils::PacketUtils::retrieve_ip_header(pkt);
        switch (h_ip.protocol) {
            /* refers to https://en.wikipedia.org/wiki/List_of_IP_protocol_numbers
             * for a complete list if other protocols are needed e.g.
             * case 1:
             *   type_ = icmp packets;
             *   break;
             */
            case 6:
                type_ = classifier::TCP;
                break;
            case 17:
                type_ = classifier::UDP;
                break;
            default:
                type_ = classifier::DEFAULT;
                break;
        }
    }
}

bool classifier::Classifier::Mapper::operator==(const Mapper & m) const {
    return type_ == m.type_;
}

bool classifier::Classifier::Mapper::operator<(const Mapper & m) const {
    return type_ < m.type_;
}

classifier::pkt_type classifier::Classifier::Mapper::default_mapping() {
    return pkt_type::DEFAULT;
}


// ************************** temporary testing stuff **************************
//TODO to remove when harbour calls are implemented
classifier::Classifier::Mapper classifier::Classifier::Mapper::defaultUDPMapper() {
    Mapper m;
    m.type_ = pkt_type::UDP;
    return m;
}

//TODO to remove when harbour calls are implemented
classifier::Classifier::Mapper classifier::Classifier::Mapper::defaultTCPMapper() {
    Mapper m;
    m.type_ = pkt_type::TCP;
    return m;
}

// TODO to refactor
void classifier::Classifier::set_remote(const char *ip, uint16_t port) {
    // TODO make requesto to harbour to populate sfc_map
    Mapper m_udp = Mapper::defaultUDPMapper();
    Mapper m_tcp = Mapper::defaultTCPMapper();

}
