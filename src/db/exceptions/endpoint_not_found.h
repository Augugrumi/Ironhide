#ifndef ENDPOINT_NOT_FOUND_H
#define ENDPOINT_NOT_FOUND_H

#include <exception>
#include <string>

namespace db {
namespace exceptions {
namespace logic_failure {
class endpoint_not_found : public std::exception
{
public:
    endpoint_not_found(const std::string&);

    const char* what() const noexcept override;
private:
    const std::string err;
};
} // namespace logic_failure
} // namespace exceptions
} // namespace db


#endif // ENDPOINT_NOT_FOUND_H
