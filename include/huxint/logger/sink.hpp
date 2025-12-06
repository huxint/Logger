#pragma once
#include <string>
#include <fstream>
#include <print>
#include <mutex>
#include <format>
#include <stdexcept>
#include <string_view>
#include "level.hpp"
#ifdef _WIN32
#include <windows.h>
#endif
#include <chrono>

namespace huxint {
    inline std::once_flag ansi_flag;
    inline void enable_ansi() {
        std::call_once(ansi_flag, [] {
#ifdef _WIN32
            const HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
            DWORD mode = 0;
            GetConsoleMode(hOut, &mode);
            SetConsoleMode(hOut, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#endif
        });
    }

    // Sink 基类
    class Sink {
    public:
        virtual ~Sink() noexcept = default;
        virtual void write(Level level, std::string_view name, const std::string &msg) = 0;
        virtual void flush() = 0;
    };

    // 控制台输出
    class ConsoleSink final : public Sink {
    public:
        explicit ConsoleSink(const bool color = true)
        : color_(color) {
            if (color_) {
                enable_ansi();
            }
        }

        void write(const Level level, std::string_view name, const std::string &msg) override {
            std::scoped_lock lock(mutex_);
            if (color_) {
                if (name.empty()) {
                    std::println("{}[{:>5}] {}{}", color_code(level), to_string(level), msg, "\033[0m");
                } else {
                    std::println("{}[{:>5}]<{}> {}{}", color_code(level), to_string(level), name, msg, "\033[0m");
                }
            } else {
                if (name.empty()) {
                    std::println("[{:>5}] {}", to_string(level), msg);
                } else {
                    std::println("[{:>5}]<{}> {}", to_string(level), name, msg);
                }
            }
        }

        void flush() override {
            std::scoped_lock lock(mutex_);
            std::fflush(stdout);
        }

    private:
        bool color_;
        std::mutex mutex_;

        static constexpr std::string_view color_code(const Level level) noexcept {
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

        void write(const Level level, std::string_view name, const std::string &msg) override {
            std::scoped_lock lock(mutex_);
            if (name.empty()) {
                file_ << std::format("[time: {:%F %T}][{:>5}] {}\n",
                                     std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now()),
                                     to_string(level),
                                     msg);
            } else {
                file_ << std::format("[time: {:%F %T}][{:>5}]<{}> {}\n",
                                     std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now()),
                                     to_string(level),
                                     name,
                                     msg);
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