#ifndef QUERY_AMBIGUOUS_H
#define QUERY_AMBIGUOUS_H

#include <exception>
#include <string>

namespace db {
namespace exceptions {
namespace logic_failure {
class query_ambiguous : public std::exception
{
public:
    query_ambiguous(const std::string&);

    const char* what() const noexcept override;
private:
    const std::string err;
};
} // namespace logic_failure
} // namespace exceptions
} // namespace db

#endif // QUERY_AMBIGUOUS_H
