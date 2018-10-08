#ifndef LOG_H
#define LOG_H

#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>

namespace utils {
class Log {
public:
    enum Level {
        trace,
        debug,
        info,
        warning,
        fatal
    };

    static Log* instance();

    void log(Level, const std::string &) const;
    void set_log_level(Level) const;
    void set_prefix(const std::string &) const;

protected:
    Log() noexcept;
    virtual ~Log();
private:
    static Log* inst;
    static Level log_level;
    static std::string prefix;

    std::string level_lookup(Level) const;
    std::string format_log(const std::string &, const std::string &) const;
};

#define LOG(lvl, msg) \
    utils::Log::instance()->log(lvl, msg);

#define ltrace \
    utils::Log::Level::trace

#define ldebug \
    utils::Log::Level::debug

#define linfo \
    utils::Log::Level::info

#define lwarn \
    utils::Log::Level::warning

#define lfatal \
    utils::Log::Level::fatal

}
#endif // LOG_H
