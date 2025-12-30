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
        log<Level::Trace>(wrapper, std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void debug(fmt_loc_wrapper<Args...> wrapper, Args &&...args) {
        log<Level::Debug>(wrapper, std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void info(fmt_loc_wrapper<Args...> wrapper, Args &&...args) {
        log<Level::Info>(wrapper, std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void warn(fmt_loc_wrapper<Args...> wrapper, Args &&...args) {
        log<Level::Warn>(wrapper, std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void error(fmt_loc_wrapper<Args...> wrapper, Args &&...args) {
        log<Level::Error>(wrapper, std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void fatal(fmt_loc_wrapper<Args...> wrapper, Args &&...args) {
        log<Level::Fatal>(wrapper, std::forward<Args>(args)...);
    }

    static void flush() {
        state_.flush();
    }

    static constexpr std::string_view name() {
        return Name;
    }

private:
    template <Level lv, typename... Args>
    static void log(fmt_loc_wrapper<Args...> wrapper, Args &&...args) {
        if (lv < level()) {
            return;
        }
        std::string msg = std::format("{}{}:{}{} {}",
                                      "\033[32m",
                                      wrapper.location().file_name(),
                                      wrapper.location().line(),
                                      reset_code(),
                                      std::format(wrapper.format(), std::forward<Args>(args)...));
        for (auto &sink : state_.sinks) {
            auto *p = sink.get();          // 确保在提交任务时，sink 还存在
            state_.pool->submit([p, msg] { // p 必须值传递，否则会导致悬空指针
                p->write(lv, Name.str(), msg);
            });
        }
    }
};

} // namespace huxint