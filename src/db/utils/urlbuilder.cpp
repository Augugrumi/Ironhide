#include "urlbuilder.h"

const std::string db::utils::URLBuilder::HTTP_PREFIX = "http";
const std::string db::utils::URLBuilder::HTTPS_PREFIX = "https";

db::utils::URLBuilder::URLBuilder() : url(), paths() {
}

db::utils::URLBuilder db::utils::URLBuilder::set_address(const Address& address) {
    url.append(address.get_URL());
    return *this;
}

db::utils::URLBuilder db::utils::URLBuilder::add_path(const char* path) {
    std::string toAdd;
    toAdd.append(path);
    if (url[url.size()-1] != *Address::PATH_SEPARATOR) {
        toAdd.append(Address::PATH_SEPARATOR);
    }
    paths.push_back(toAdd);
    return *this;
}

db::utils::URLBuilder db::utils::URLBuilder::add_path(const std::string& path) {
    add_path(path.c_str());
    return *this;
}

std::string db::utils::URLBuilder::build() const {
    std::string toBuild;
    toBuild.append(url);

    for (auto it = paths.cbegin(); it != paths.cend(); it++) {
        toBuild.append(*it);
    }
    return toBuild;
}
