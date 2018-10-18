//
// Created by zanna on 08/10/18.
//

#ifndef IRONHIDE_CONNECTIONENTRY_H
#define IRONHIDE_CONNECTIONENTRY_H

#include <string>
#include "dbquery.h"

namespace endpoint {

class ConnectionEntry {
private:
    const std::string ip_src_;
    const std::string ip_dst_;
    const uint16_t port_src_;
    const uint16_t port_dst_;
    const std::string sfcid_;
    const db::protocol_type prt_;
public:
    ConnectionEntry(const std::string& ip_src,
                    const std::string& ip_dst,
                    const uint16_t & port_src,
                    const uint16_t & port_dst,
                    const std::string & sfcid,
                    const db::protocol_type& prt);
    bool operator==(const ConnectionEntry& ce) const;
    bool operator<(const ConnectionEntry& ce) const;
    const std::string &get_ip_src() const;
    const std::string &get_ip_dst() const;
    uint16_t get_port_src() const;
    uint16_t get_port_dst() const;
    std::string get_sfcid() const;
    db::protocol_type get_protocol_type() const;
};

}

#endif //IRONHIDE_CONNECTIONENTRY_H
