#include "log.h"

namespace utils {
Log* Log::inst = new Log();
Log::Level Log::log_level = Log::Level::info;
std::string Log::prefix = "";

std::string Log::level_lookup(Level to_lookup) const {
    std::string res;
    switch (to_lookup) {
    case Level::trace : res = "trace"; break;
    case Level::debug : res = "debug"; break;
    case Level::info : res = "info"; break;
    case Level::warning : res = "warning"; break;
    case Level::fatal : res = "fatal"; break;
    }

    return res;
}

std::string Log::format_log(
        const std::string & level,
        const std::string & message) const {

    // get current time
   auto now = std::chrono::system_clock::now();

   // get number of milliseconds for the current second
   // (remainder after division into seconds)
   auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
               now.time_since_epoch()
               ) % 1000;

   // convert to std::time_t in order to convert to std::tm (broken time)
   auto timer = std::chrono::system_clock::to_time_t(now);

   // convert to broken time
   std::tm bt = *std::localtime(&timer);

   std::ostringstream oss;
   oss << std::put_time(&bt, "%H:%M:%S"); // HH:MM:SS
   oss << '.' << std::setfill('0') << std::setw(3) << ms.count();
   std::string time = oss.str();

   std::string timestamp =  std::to_string(bt.tm_mday) + "/" +
           std::to_string(bt.tm_mon) + "/" +
           std::to_string(bt.tm_year + 1900) + " - " +
           time;

   std::string complete_prefix = prefix != "" ? prefix + " - " : "";
   return complete_prefix +
           "[" + timestamp + "] - " +
           "[" + level + "] - " +
           message;
}

void Log::log(Level level, const std::string & to_print) const {
    if (level >= log_level) {
        if (level == Log::Level::fatal) {
            std::cerr << format_log(level_lookup(level), to_print) << std::endl;
        } else {
            std::cout << format_log(level_lookup(level), to_print) << std::endl;
        }
    }
}

void Log::set_log_level(Level new_log_level) const {
    log_level = new_log_level;
}

void Log::set_prefix(const std::string & new_prefix) const {
    prefix = new_prefix;
}

Log::Log() noexcept {
}

Log::~Log() {
}

Log* Log::instance() {
    return inst;
}
}
