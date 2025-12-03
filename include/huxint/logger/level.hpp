#pragma once
#include <cstdint>
#include <string>

// 日志级别
namespace huxint {
    enum class Level : std::uint8_t { Trace, Debug, Info, Warn, Error, Fatal };
    inline std::string to_string(const Level level) {
        switch (level) {
            case Level::Trace:
                return "TRACE";
            case Level::Debug:
                return "DEBUG";
            case Level::Info:
                return "INFO";
            case Level::Warn:
                return "WARN";
            case Level::Error:
                return "ERROR";
            case Level::Fatal:
                return "FATAL";
            default:
                return "UNKNOWN";
        }
    }
} // namespace huxint