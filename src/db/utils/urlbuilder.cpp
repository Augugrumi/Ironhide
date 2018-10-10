#include "urlbuilder.h"
#include "log.h"

const std::string db::utils::URLBuilder::HTTP_PREFIX = "http";
const std::string db::utils::URLBuilder::HTTPS_PREFIX = "https";

db::utils::URLBuilder::URLBuilder() : url(), paths() {
}

db::utils::URLBuilder db::utils::URLBuilder::set_address(const Address& address) {
    url.append(address.get_URL());
    return *this;
}

db::utils::URLBuilder db::utils::URLBuilder::add_path(const char* path) {
    std::string to_add;
    if (paths.size() != 0 && path[0] != *Address::PATH_SEPARATOR) {
        to_add.append(Address::PATH_SEPARATOR);
    }
    to_add.append(path);
    paths.push_back(to_add);
    return *this;
}

db::utils::URLBuilder db::utils::URLBuilder::add_path(const std::string& path) {
    add_path(path.c_str());
    return *this;
}

std::string db::utils::URLBuilder::build() const {
    std::string to_build;
    to_build.append(url);

    for (auto it = paths.cbegin(); it != paths.cend(); it++) {
        to_build.append(*it);
    }
    return to_build;
}
