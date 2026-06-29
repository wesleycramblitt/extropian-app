#include <cstdio>
#include <cstdarg>
#include <string>
#include <chrono>
#include <ctime>
#include <vector>
#include <mutex>

namespace exd::app::services {

enum class LogLevel { Debug, Info, Warning, Error };

struct LogService {
    struct Entry {
        LogLevel level;
        std::string message;
        std::string timestamp;
    };
    std::vector<Entry> entries;
    std::mutex mtx;

    void log(LogLevel lv, const char* fmt, va_list args) {
        char buf[1024];
        std::vsnprintf(buf, sizeof(buf), fmt, args);
        auto now = std::chrono::system_clock::now();
        auto t = std::chrono::system_clock::to_time_t(now);
        char ts[32];
        std::strftime(ts, sizeof(ts), "%H:%M:%S", std::localtime(&t));
        std::lock_guard<std::mutex> lock(mtx);
        entries.push_back({lv, buf, ts});
        std::fprintf((lv >= LogLevel::Error) ? stderr : stdout,
                     "[%s] %s\n", ts, buf);
    }

    void debug(const char* fmt, ...) { va_list a; va_start(a, fmt); log(LogLevel::Debug, fmt, a); va_end(a); }
    void info(const char* fmt, ...)  { va_list a; va_start(a, fmt); log(LogLevel::Info, fmt, a); va_end(a); }
    void warn(const char* fmt, ...)  { va_list a; va_start(a, fmt); log(LogLevel::Warning, fmt, a); va_end(a); }
    void error(const char* fmt, ...) { va_list a; va_start(a, fmt); log(LogLevel::Error, fmt, a); va_end(a); }
    void clear() { std::lock_guard<std::mutex> lock(mtx); entries.clear(); }
};

static LogService& instance() { static LogService s; return s; }
void log_debug(const char* fmt, ...) { va_list a; va_start(a, fmt); instance().log(LogLevel::Debug, fmt, a); va_end(a); }
void log_info(const char* fmt, ...)  { va_list a; va_start(a, fmt); instance().log(LogLevel::Info, fmt, a); va_end(a); }
void log_warn(const char* fmt, ...)  { va_list a; va_start(a, fmt); instance().log(LogLevel::Warning, fmt, a); va_end(a); }
void log_error(const char* fmt, ...) { va_list a; va_start(a, fmt); instance().log(LogLevel::Error, fmt, a); va_end(a); }

} // namespace exd::app::services
