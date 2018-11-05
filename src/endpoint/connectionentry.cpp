#include "connectionentry.h"

endpoint::ConnectionEntry::ConnectionEntry(const std::string &ip_src,
                                           const std::string &ip_dst,
                                           const uint16_t &port_src,
                                           const uint16_t &port_dst,
                                           const std::string &sfcid,
                                           const db::protocol_type &prt)
        : ip_src_(ip_src), ip_dst_(ip_dst),
          port_src_(port_src), port_dst_(port_dst),
          sfcid_(sfcid), prt_(prt) {}

bool endpoint::ConnectionEntry::operator==(
        const endpoint::ConnectionEntry &ce) const {
    return prt_ == ce.prt_ &&
           ip_src_ == ce.ip_src_ &&
           ip_dst_ == ce.ip_dst_ &&
           port_src_ == ce.port_src_ &&
           port_dst_ == ce.port_dst_ &&
           sfcid_ == ce.sfcid_;
}

bool endpoint::ConnectionEntry::operator<(
        const endpoint::ConnectionEntry &ce) const {
    // workaround to use with maps, it does mean nothing operator < for this class
    return prt_ < ce.prt_ ||
           ip_src_ < ce.ip_src_ ||
           ip_dst_ < ce.ip_dst_ ||
           port_src_ < ce.port_src_ ||
           port_dst_ < ce.port_dst_ ||
           sfcid_ < ce.sfcid_;
}

const std::string &endpoint::ConnectionEntry::get_ip_src() const {
    return ip_src_;
}

const std::string &endpoint::ConnectionEntry::get_ip_dst() const {
    return ip_dst_;
}

uint16_t endpoint::ConnectionEntry::get_port_src() const {
    return port_src_;
}

uint16_t endpoint::ConnectionEntry::get_port_dst() const {
    return port_dst_;
}

std::string endpoint::ConnectionEntry::get_sfcid() const {
    return sfcid_;
}

db::protocol_type endpoint::ConnectionEntry::get_protocol_type() const {
    return prt_;
}
