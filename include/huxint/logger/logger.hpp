#pragma once
#include <string>
#include <string_view>
#include <memory>
#include <format>
#include <vector>
#include <concepts>
#include "level.hpp"
#include "sink.hpp"
#include "util.hpp"
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

    static void level(const Level lv) {
        state_.level = lv;
    }

    static Level level() {
        return state_.level;
    }

    static void set_thread_count(std::size_t count) {
        state_.pool = std::make_unique<ThreadPool<>>(count);
    }

    template <typename... Args>
    static void trace(fmt_loc_wrapper<Args...> wrapper, Args &&...args) {
        format<Level::Trace, true>(wrapper, std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void debug(fmt_loc_wrapper<Args...> wrapper, Args &&...args) {
        format<Level::Debug, true>(wrapper, std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void info(fmt_loc_wrapper<Args...> wrapper, Args &&...args) {
        format<Level::Info, true>(wrapper, std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void warn(fmt_loc_wrapper<Args...> wrapper, Args &&...args) {
        format<Level::Warn, true>(wrapper, std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void error(fmt_loc_wrapper<Args...> wrapper, Args &&...args) {
        format<Level::Error, true>(wrapper, std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void fatal(fmt_loc_wrapper<Args...> wrapper, Args &&...args) {
        format<Level::Fatal, true>(wrapper, std::forward<Args>(args)...);
    }

    // 不带文件名和行号的版本
    template <typename... Args>
    static void trace_raw(std::format_string<Args...> fmt, Args &&...args) {
        format<Level::Trace, false>(fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void debug_raw(std::format_string<Args...> fmt, Args &&...args) {
        format<Level::Debug, false>(fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void info_raw(std::format_string<Args...> fmt, Args &&...args) {
        format<Level::Info, false>(fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void warn_raw(std::format_string<Args...> fmt, Args &&...args) {
        format<Level::Warn, false>(fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void error_raw(std::format_string<Args...> fmt, Args &&...args) {
        format<Level::Error, false>(fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void fatal_raw(std::format_string<Args...> fmt, Args &&...args) {
        format<Level::Fatal, false>(fmt, std::forward<Args>(args)...);
    }

    static void flush() {
        state_.flush();
    }

    static constexpr std::string_view name() {
        return Name;
    }

private:
    template <Level lv, bool Location, typename Fmt, typename... Args>
    static void format(Fmt &&fmt, Args &&...args) {
        if (lv < level()) {
            return;
        }
        std::string msg;
        if constexpr (Location) {
            msg = std::format("{}{}:{}{} {}",
                              "\033[32m",
                              fmt.location().file_name(),
                              fmt.location().line(),
                              reset_code(),
                              std::format(fmt.format(), std::forward<Args>(args)...));
        } else {
            msg = std::format(std::forward<Fmt>(fmt), std::forward<Args>(args)...);
        }
        for (auto &sink : state_.sinks) {
            auto *p = sink.get();
            state_.pool->submit([p, msg] {
                p->write(lv, Name.str(), msg);
            });
        }
    }
};

} // namespace huxint