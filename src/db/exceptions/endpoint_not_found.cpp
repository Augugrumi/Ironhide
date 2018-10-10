#include "endpoint_not_found.h"

db::exceptions::logic_failure::endpoint_not_found::endpoint_not_found(const std::string& str_err)
    : err(str_err) {
}

const char* db::exceptions::logic_failure::endpoint_not_found::what()
const noexcept {
    return err.c_str();
}
