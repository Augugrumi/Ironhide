//
// Created by zanna on 05/10/18.
//

#ifndef IRONHIDE_CLASSIFICATOR_H
#define IRONHIDE_CLASSIFICATOR_H

#include <cstdio>
#include <map>

namespace classifier {

// TODO think about how to classify results
enum pkt_type {DEFAULT, TCP, UDP};

class Classifier {
public:
    class Mapper;
private:
    // TODO change map type
    // TODO request sfcs in order to popolate the map
    std::map<Mapper, char*> sfc_map;
public:
    class Mapper {
    private:
        pkt_type type_;
    public:
        void set_pkt(unsigned char * pkt,
                     size_t pkt_len);
        bool operator==(const Mapper&) const;
        bool operator<(const Mapper&) const;
        static pkt_type default_mapping();
    };
    const char* classify_pkt(unsigned char * pkt,
                             size_t pkt_len);
};

} // namespace classifier

#endif //IRONHIDE_CLASSIFICATOR_H
