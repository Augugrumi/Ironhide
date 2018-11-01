#include "failure.h"

db::exceptions::ios_base::failure::failure(const std::string &str_err)
        : err(str_err) {
}

const char *db::exceptions::ios_base::failure::what() const noexcept {
    return err.c_str();
}
