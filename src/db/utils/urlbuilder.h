#ifndef URLBUILDER_H
#define URLBUILDER_H

#include <string>
#include <vector>

#include "address.h"

namespace db {
namespace utils {
class URLBuilder
{
public:
    URLBuilder();

    URLBuilder add_path(const char*);
    URLBuilder add_path(const std::string&);

    URLBuilder set_address(const Address&);

    std::string build() const;
private:
    const static std::string HTTP_PREFIX;
    const static std::string HTTPS_PREFIX;
    std::string url;
    std::vector<std::string> paths;
};
} // namespace utils
} // namespace db

#endif // URLBUILDER_H
