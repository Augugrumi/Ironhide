//
// Created by zanna on 08/10/18.
//

#ifndef IRONHIDE_CONNECTIONENTRY_H
#define IRONHIDE_CONNECTIONENTRY_H

#include <string>

namespace endpoint {

class ConnectionEntry {
private:
    const std::string ip_src_;
    const std::string ip_dst_;
    const uint16_t port_src_;
    const uint16_t port_dst_;
    const std::string sfcid_;
public:
    ConnectionEntry(const std::string& ip_src,
                    const std::string& ip_dst,
                    const uint16_t & port_src,
                    const uint16_t & port_dst,
                    const std::string & sfcid);
    bool operator==(const ConnectionEntry& ce) const;
    bool operator<(const ConnectionEntry& ce) const;
    const std::string &get_ip_src() const;
    const std::string &get_ip_dst() const;
    uint16_t get_port_src() const;
    uint16_t get_port_dst() const;
    const std::string &get_sfcid() const;
};

}

#endif //IRONHIDE_CONNECTIONENTRY_H
