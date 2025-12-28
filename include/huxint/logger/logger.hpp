#pragma once
#include <string>
#include <string_view>
#include <memory>
#include <format>
#include <vector>
#include <concepts>
#include "level.hpp"
#include "sink.hpp"
#include <huxint/thread_pool.hpp>

namespace huxint {
// Logger 状态
struct LoggerState {
    Level level = Level::Trace;
    std::vector<std::unique_ptr<Sink>> sinks;
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
    template <typename T, typename... Args>
        requires std::derived_from<T, Sink>
    static T *add_sink(Args &&...args) { // 添加返回 sink 指针, 方便自己配置
        auto sink = std::make_unique<T>(std::forward<Args>(args)...);
        auto *ptr = sink.get();
        state_.sinks.push_back(std::move(sink));
        return ptr;
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
            auto *p = sink.get();                 // 确保在提交任务时，sink 还存在
            state_.pool->submit([p, level, msg] { // p 必须值传递，否则会导致悬空指针
                p->write(level, Name.str(), msg);
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