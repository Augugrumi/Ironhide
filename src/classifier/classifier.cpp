//
// Created by zanna on 05/10/18.
//

#include "classifier.h"

const char * classifier::Classifier::classify_pkt(unsigned char *pkt,
                                                  size_t pkt_len) {
    classifier::Classifier::Mapper m;
    m.set_pkt(pkt, pkt_len);
    return sfc_map.at(m);
}

void classifier::Classifier::Mapper::set_pkt(unsigned char *pkt,
                                             size_t pkt_len) {
    //TODO take the packet, check the header and at least classify based on
    //     protocol. At the end set the type_ to not save all the pkt
    //     do not know how to do? default chain
    type_ = classifier::DEFAULT;
}

bool classifier::Classifier::Mapper::operator==(
        const classifier::Classifier::Mapper & m) const {
    return type_ == m.type_;
}

classifier::pkt_type classifier::Classifier::Mapper::default_mapping() {
    return pkt_type::DEFAULT;
}
