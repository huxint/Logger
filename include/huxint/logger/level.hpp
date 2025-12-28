#pragma once
#include <cstdint>
#include <string_view>
#include <utility>

// 日志级别
namespace huxint {
enum class Level : std::uint8_t { Trace, Debug, Info, Warn, Error, Fatal };

constexpr std::string_view to_string(const Level level) noexcept {
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
            std::unreachable();
            return "UNKNOWN";
    }
}
} // namespace huxint