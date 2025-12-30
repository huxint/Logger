#pragma once
#include "level.hpp"
#include <algorithm>
#include <format>
#include <source_location>
#ifdef WIN32
#include <windows.h>
#endif

// 颜色代码
namespace huxint {
// 编译期字符串
template <std::size_t N>
struct String {
    char data[N]{};
    constexpr String(const char (&str)[N]) {
        std::copy_n(str, N, data);
    }

    constexpr operator std::string_view() const {
        return {data, N - 1};
    }

    constexpr std::string_view str() const {
        return {data, N - 1};
    }
};

constexpr std::string_view color_code(const Level level) noexcept {
    switch (level) {
        case Level::Trace:
            return "\033[100m"; // gray
        case Level::Debug:
            return "\033[44m"; // cyan
        case Level::Info:
            return "\033[42m"; // green
        case Level::Warn:
            return "\033[43m"; // yellow
        case Level::Error:
            return "\033[41m"; // red
        case Level::Fatal:
            return "\033[45m"; // magenta
        default:
            std::unreachable();
            return "NOT DEFINED COLOR";
    }
}

constexpr std::string_view reset_code() noexcept {
    return "\033[0m";
}

constexpr auto here(std::source_location location = std::source_location::current()) -> std::source_location {
    return location;
}
} // namespace huxint