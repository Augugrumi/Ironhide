#include "urlbuilder.h"

db::utils::URLBuilder::URLBuilder() : url(), paths() {
}

db::utils::URLBuilder &
db::utils::URLBuilder::set_address(const Address &address) {
    url.append(address.get_URL());
    return *this;
}

db::utils::URLBuilder &db::utils::URLBuilder::add_path(const char *path) {
    std::string to_add;
    if (!paths.empty() &&
        // Last character of the last element of the vector
        paths[paths.size() - 1][paths[paths.size() - 1].size() - 1] !=
        *Address::PATH_SEPARATOR &&
        path[0] != *Address::PATH_SEPARATOR) {
        to_add.append(Address::PATH_SEPARATOR);
    }

    to_add.append(path);
    paths.push_back(to_add);

    return *this;
}

db::utils::URLBuilder &
db::utils::URLBuilder::add_path(const std::string &path) {
    return add_path(path.c_str());
}

std::string db::utils::URLBuilder::build() const {
    std::string to_build;
    to_build.append(url);

    for (const auto &path : paths) {
        to_build.append(path);
    }
    return to_build;
}
