#pragma once

#include "util/singleton.hpp"
#include <deque>
#include <string>
#include <cstdint>

enum class LogLevel : uint8_t {
    TRACE,
    INFO,
    WARN,
    ERROR,
    SUCCESS,
};

struct LogEntry {
    LogLevel level;
    std::string message;
    uint64_t id;
};

class Logger {
    MAKE_SINGLETON(Logger);
private:
    static constexpr size_t MAX_HISTORY_SIZE = 512;
    
    static std::string LevelToString(const LogLevel &level);
    static std::string GetTimestamp();

    uint64_t messageCount;
    std::deque<LogEntry> history;

public:
    void Log(const LogLevel &level, const char *format, ...);

    [[nodiscard]] inline uint64_t GetMessageCount() const {
        return messageCount;
    }

    [[nodiscard]] inline const std::deque<LogEntry> &GetHistory() const {
        return history;
    }
};
