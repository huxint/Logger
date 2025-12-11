#pragma once
#include "level.hpp"
#include <algorithm>
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
                return "\033[90m"; // gray
            case Level::Debug:
                return "\033[36m"; // cyan
            case Level::Info:
                return "\033[32m"; // green
            case Level::Warn:
                return "\033[33m"; // yellow
            case Level::Error:
                return "\033[31m"; // red
            case Level::Fatal:
                return "\033[35m"; // magenta
            default:
                return "\033[0m";
        }
    }

} // namespace huxint