#include "address.h"

const char *const db::utils::Address::PORT_SEPARATOR = ":";
const char *const db::utils::Address::PATH_SEPARATOR = "/";
const std::string db::utils::Address::HTTP_PREFIX("http://");

db::utils::Address::Address(const std::string &new_address, uint16_t new_port)
        : address(new_address), port(new_port), has_http_prefix(false) {
    has_http_prefix = prefix_check(new_address);
}

db::utils::Address::Address(const std::string &complete_address) {
    has_http_prefix = prefix_check(complete_address);
    if (!has_http_prefix) {
        address_parse(complete_address);
    } else {
        size_t found = complete_address.find_first_of(PATH_SEPARATOR);
        std::string protocol = complete_address.substr(0, found);
        std::string url_new = complete_address.substr(found + 3);
        address_parse(url_new);
    }
}

void db::utils::Address::address_parse(const std::string &url) {
    size_t found = url.find_first_of(PORT_SEPARATOR);
    address = url.substr(0, found);
    size_t found1 = url.find_first_of(PATH_SEPARATOR);
    port = (uint16_t) (std::stoul(url.substr(found + 1, found1 - found - 1)));
}

bool db::utils::Address::prefix_check(const std::string &address) const {
    return strncmp(address.c_str(), HTTP_PREFIX.c_str(), HTTP_PREFIX.size()) ==
           0;
}

int db::utils::Address::get_port() const {
    return port;
}

std::string db::utils::Address::get_address() const {
    return address;
}

std::string db::utils::Address::get_URL() const {
    std::string res;
    if (!has_http_prefix) {
        res.append(HTTP_PREFIX);
    }
    res.append(address)
            .append(PORT_SEPARATOR)
            .append(std::to_string(port))
            .append(PATH_SEPARATOR);

    return res;
}
