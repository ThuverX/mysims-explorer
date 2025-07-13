#pragma once

#include "macros/singleton.hpp"
#include <deque>
#include <string>

enum class LogLevel {
    DEBUG,
    INFO,
    WARN,
    ERROR
};

struct LogEntry {
    LogLevel level;
    std::string message;
};

class Logger {
    MAKE_SINGLETON(Logger);
private:
    static constexpr size_t MAX_HISTORY_SIZE = 512;

    static std::string LevelToString(const LogLevel level);
    static std::string GetTimestamp();

    std::deque<LogEntry> history;

public:
    void Log(const LogLevel level, const char *format, ...);

    inline const std::deque<LogEntry> &GetHistory() const {
        return history;
    }
};
