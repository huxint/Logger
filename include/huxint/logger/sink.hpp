#pragma once
#include <string>
#include <string_view>
#include <fstream>
#include <print>
#include <format>
#include <stdexcept>
#include <chrono>
#include <mutex>
#include "level.hpp"
#include "util.hpp"

namespace huxint {
// 虚基类，用于运行时多态
class Sink {
public:
    virtual ~Sink() = default;
    virtual void write(Level level,
                       std::string_view name,
                       const std::string &msg,
                       std::string_view file = "",
                       std::uint32_t line = 0) = 0;
    virtual void flush() = 0;
};

// 控制台输出
template <bool Color = true>
class ConsoleSink final : public Sink {
public:
    explicit ConsoleSink() {
#ifdef _WIN32
        if constexpr (Color) {
            const HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
            DWORD mode = 0;
            GetConsoleMode(hOut, &mode);
            SetConsoleMode(hOut, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
        }
#endif
    }

    void write(Level level,
               std::string_view name,
               const std::string &msg,
               std::string_view file = "",
               std::uint32_t line = 0) override {
        std::scoped_lock lock(mutex_);
        if constexpr (Color) {
            if (name.empty()) {
                if (file.empty()) {
                    std::println("{}[{:>5}]{} {}", color_code(level), to_string(level), reset_code(), msg);
                } else {
                    std::println("{}[{:>5}]{} {}{}:{}{} {}",
                                 color_code(level),
                                 to_string(level),
                                 reset_code(),
                                 "\033[32m",
                                 file,
                                 line,
                                 reset_code(),
                                 msg);
                }
            } else {
                if (file.empty()) {
                    std::println("{}[{:>5}]<{}>{} {}", color_code(level), to_string(level), name, reset_code(), msg);
                } else {
                    std::println("{}[{:>5}]<{}>{} {}{}:{}{} {}",
                                 color_code(level),
                                 to_string(level),
                                 name,
                                 reset_code(),
                                 "\033[32m",
                                 file,
                                 line,
                                 reset_code(),
                                 msg);
                }
            }
        } else {
            if (name.empty()) {
                if (file.empty()) {
                    std::println("[{:>5}] {}", to_string(level), msg);
                } else {
                    std::println("[{:>5}] {}:{} {}", to_string(level), file, line, msg);
                }
            } else {
                if (file.empty()) {
                    std::println("[{:>5}]<{}> {}", to_string(level), name, msg);
                } else {
                    std::println("[{:>5}]<{}> {}:{} {}", to_string(level), name, file, line, msg);
                }
            }
        }
    }

    void flush() override {
        std::scoped_lock lock(mutex_);
        std::fflush(stdout);
    }

private:
    std::mutex mutex_;
};

// 文件输出
class FileSink final : public Sink {
public:
    explicit FileSink(const std::string &filename)
    : file_(filename, std::ios::app) {
        if (!file_.is_open()) {
            throw std::runtime_error("Failed to open log file: " + filename);
        }
    }

    void write(Level level,
               std::string_view name,
               const std::string &msg,
               std::string_view file = "",
               std::uint32_t line = 0) override {
        std::scoped_lock lock(mutex_);
        auto now = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
        if (name.empty()) {
            if (file.empty()) {
                file_ << std::format("[time: {:%F %T}][{:>5}] {}\n", now, to_string(level), msg);
            } else {
                file_ << std::format("[time: {:%F %T}][{:>5}] {}:{} {}\n", now, to_string(level), file, line, msg);
            }
        } else {
            if (file.empty()) {
                file_ << std::format("[time: {:%F %T}][{:>5}]<{}> {}\n", now, to_string(level), name, msg);
            } else {
                file_ << std::format(
                    "[time: {:%F %T}][{:>5}]<{}> {}:{} {}\n", now, to_string(level), name, file, line, msg);
            }
        }
    }

    void flush() override {
        std::scoped_lock lock(mutex_);
        file_.flush();
    }

private:
    std::ofstream file_;
    std::mutex mutex_;
};
} // namespace huxint