//
// Created by zanna on 05/10/18.
//

#ifndef IRONHIDE_ADDRESS_H
#define IRONHIDE_ADDRESS_H

#include <string>
#include <cstring>
#include <cstdint>

namespace db {
namespace utils {
class Address
{
public:
    Address(const std::string&, uint16_t);
    explicit Address(const std::string&);

    const static char* const PATH_SEPARATOR;
    const static char* const PORT_SEPARATOR;

    int get_port() const;
    std::string get_address() const;

    std::string get_URL() const;
private:
    const static std::string HTTP_PREFIX;
    std::string address;
    uint16_t port;
    bool has_http_prefix;

    bool prefix_check(const std::string&) const;
    void address_parse(const std::string&);
};
} // namespace address
} // namespace utils


#endif //IRONHIDE_ADDRESS_H
