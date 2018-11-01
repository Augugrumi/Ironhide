#include "query_ambiguous.h"

db::exceptions::logic_failure::query_ambiguous::query_ambiguous(
        const std::string &str_err) : err(str_err) {
}

const char *
db::exceptions::logic_failure::query_ambiguous::what() const noexcept {
    return err.c_str();
}
