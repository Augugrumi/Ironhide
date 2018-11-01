//
// Created by zanna on 08/10/18.
//

#ifndef IRONHIDE_CONNECTIONENTRY_H
#define IRONHIDE_CONNECTIONENTRY_H

#include <string>
#include "dbquery.h"

namespace endpoint {

/**
 * Class used to contain data on a connection
 */
class ConnectionEntry {
private:
    /**
     * IP of the sender
     */
    const std::string ip_src_;
    /**
     * IP of the destination
     */
    const std::string ip_dst_;
    /**
     * Port of the sender
     */
    const uint16_t port_src_;
    /**
     * Port of the destination
     */
    const uint16_t port_dst_;
    /**
     * ID of the SFC
     */
    const std::string sfcid_;
    /**
     * TCP or UDP
     */
    const db::protocol_type prt_;
public:
    ConnectionEntry(const std::string& ip_src,
                    const std::string& ip_dst,
                    const uint16_t & port_src,
                    const uint16_t & port_dst,
                    const std::string & sfcid,
                    const db::protocol_type& prt);
    bool operator==(const ConnectionEntry& ce) const;
    /**
     * Method implemented only to make possible to use ConnectionEntry with
     * std::map
     */
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
