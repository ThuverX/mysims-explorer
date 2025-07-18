#include "Logger.hpp"

#include <cstdio>
#include <cstdarg>
#include <iostream>
#include <chrono>
#include <sstream>
#include <iomanip>

std::string Logger::LevelToString(const LogLevel &level) {
    switch (level) {
        case LogLevel::TRACE: return "TRACE  ";
        case LogLevel::INFO:  return "INFO   ";
        case LogLevel::WARN:  return "WARN   ";
        case LogLevel::ERROR: return "ERROR  ";
        case LogLevel::SUCCESS: return "SUCCESS";
    }
    return "UNKNOWN";
}

std::string Logger::GetTimestamp() {
    using namespace std::chrono;

    auto now = system_clock::now();
    std::time_t timeT = system_clock::to_time_t(now);
    
    std::tm tm{}; // NOLINT(readability-identifier-length)
    if (localtime_s(&tm, &timeT) != 0) {
        return {};
    }

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

void Logger::Log(const LogLevel &level, const char *format, ...) {
    constexpr size_t BUFFER_SIZE = 1024;
    char buffer[BUFFER_SIZE];

    va_list args;
    va_start(args, format);
    int size = vsnprintf(buffer, BUFFER_SIZE, format, args);
    va_end(args);

    std::string timestamp = GetTimestamp();
    std::string levelStr = LevelToString(level);
    
    std::string message(buffer, size);

    LogEntry entry = {
        level,
        "[" + timestamp + "][" + levelStr + "] " + message,
        messageCount,
    };

    messageCount++;

    switch (level) {
        case LogLevel::WARN:
        case LogLevel::ERROR:
            std::cerr << entry.message << "\n";
            break;
        default:
            std::cout << entry.message << "\n";
            break;
    }

    if (history.size() + 1 > MAX_HISTORY_SIZE) {
        history.pop_front();
    }
    history.emplace_back(entry);
}
