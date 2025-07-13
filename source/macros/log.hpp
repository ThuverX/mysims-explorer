#pragma once

#include "../Logger.hpp"

#define LOG_DEBUG(message, ...) Logger::Get().Log(LogLevel::DEBUG, message, ##__VA_ARGS__)
#define LOG_INFO(message, ...)  Logger::Get().Log(LogLevel::INFO,  message, ##__VA_ARGS__)
#define LOG_WARN(message, ...)  Logger::Get().Log(LogLevel::WARN,  message, ##__VA_ARGS__)
#define LOG_ERROR(message, ...) Logger::Get().Log(LogLevel::ERROR, message, ##__VA_ARGS__)
