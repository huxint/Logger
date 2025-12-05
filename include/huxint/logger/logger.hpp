#pragma once
#include <string>
#include <vector>
#include <memory>
#include <format>
#include <algorithm>
#include "level.hpp"
#include "sink.hpp"
#include "thread_pool.hpp"

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
    };

    // Logger 状态
    struct LoggerState {
        Level level = Level::Trace;
        std::vector<std::shared_ptr<Sink>> sinks;
        std::unique_ptr<ThreadPool<>> pool = std::make_unique<ThreadPool<>>(1);

        void flush() {
            pool->wait();
            for (const auto &sink : sinks) {
                sink->flush();
            }
        }

        ~LoggerState() {
            flush();
        }
    };

    template <String Name = "">
    class Logger {
        inline static LoggerState state_; // 封装, 方便析构

    public:
        static void add_sink(std::shared_ptr<Sink> sink) {
            state_.sinks.push_back(std::move(sink));
        }

        static void set_level(const Level level) {
            state_.level = level;
        }

        static Level level() {
            return state_.level;
        }

        static void set_thread_count(std::size_t count) {
            state_.pool = std::make_unique<ThreadPool<>>(count);
        }

        template <typename... Args>
        static void trace(std::format_string<Args...> fmt, Args &&...args) {
            log(Level::Trace, std::format(fmt, std::forward<Args>(args)...));
        }

        template <typename... Args>
        static void debug(std::format_string<Args...> fmt, Args &&...args) {
            log(Level::Debug, std::format(fmt, std::forward<Args>(args)...));
        }

        template <typename... Args>
        static void info(std::format_string<Args...> fmt, Args &&...args) {
            log(Level::Info, std::format(fmt, std::forward<Args>(args)...));
        }

        template <typename... Args>
        static void warn(std::format_string<Args...> fmt, Args &&...args) {
            log(Level::Warn, std::format(fmt, std::forward<Args>(args)...));
        }

        template <typename... Args>
        static void error(std::format_string<Args...> fmt, Args &&...args) {
            log(Level::Error, std::format(fmt, std::forward<Args>(args)...));
        }

        template <typename... Args>
        static void fatal(std::format_string<Args...> fmt, Args &&...args) {
            log(Level::Fatal, std::format(fmt, std::forward<Args>(args)...));
        }

        static void log(Level level, const std::string &msg) {
            if (level < state_.level) {
                return;
            }
            for (auto &sink : state_.sinks) {
                state_.pool->submit([sink, level, msg] {
                    sink->write(level, Name, msg);
                });
            }
        }

        static void flush() {
            state_.flush();
        }

        static constexpr std::string_view name() {
            return Name;
        }
    };

} // namespace huxint