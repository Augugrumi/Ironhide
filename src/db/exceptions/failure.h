#ifndef FAILURE_H
#define FAILURE_H

#include <exception>
#include <string>

namespace exceptions {
namespace ios_base {
class failure : public std::exception {
public:
    failure(const std::string&);
    ~failure() = default;

    const char* what() const noexcept override;
private:
    const std::string& err;
};
} // namespace ios_base
} // namespace exception


#endif // FAILURE_H
