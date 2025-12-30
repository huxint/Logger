#include <huxint/logger.hpp>
#include <cassert>
#include <chrono>
#include <print>
#include <thread>
#include <vector>

namespace huxint {

class MemorySink final : public Sink {
public:
    void write(Level level,
               std::string_view name,
               const std::string &msg,
               std::string_view file,
               std::uint32_t line) override {
        std::scoped_lock lock(mutex_);
        logs_.push_back({level, std::string(name), msg, std::string(file), line});
    }

    void flush() override {}

    struct Entry {
        Level level;
        std::string name;
        std::string msg;
        std::string file;
        std::uint32_t line;
    };

    auto size() {
        std::scoped_lock lock(mutex_);
        return logs_.size();
    }

    auto count(Level lv) {
        std::scoped_lock lock(mutex_);
        return std::ranges::count_if(logs_, [lv](auto &e) {
            return e.level == lv;
        });
    }

    auto &logs() {
        return logs_;
    }

    bool has_location() {
        std::scoped_lock lock(mutex_);
        return !logs_.empty() && !logs_[0].file.empty();
    }

private:
    std::vector<Entry> logs_;
    std::mutex mutex_;
};

} // namespace huxint

// 测试框架
struct Test {
    const char *name;
    bool (*fn)();
};

template <typename Logger>
void run_threads(int n, int logs_per_thread, auto &&log_fn) {
    std::vector<std::jthread> threads;
    threads.reserve(n);
    for (int t = 0; t < n; ++t) {
        threads.emplace_back([=] {
            for (int i = 0; i < logs_per_thread; ++i) {
                log_fn(t, i);
            }
        });
    }
}

// 1. 并发写入测试
bool test_concurrent() {
    using L = huxint::Logger<"Concurrent">;
    auto *sink = L::add_sink<huxint::MemorySink>();

    run_threads<L>(10, 100, [](int t, int i) {
        L::info_raw("T{} I{}", t, i);
    });
    L::flush();

    return sink->size() == 1000;
}

// 2. 日志级别测试
bool test_levels() {
    using L = huxint::Logger<"Levels">;
    auto *sink = L::add_sink<huxint::MemorySink>();
    constexpr int n = 50;

    std::jthread t1([] {
        for (int i = 0; i < n; ++i) {
            L::trace_raw("{}", i);
        }
    });
    std::jthread t2([] {
        for (int i = 0; i < n; ++i) {
            L::debug_raw("{}", i);
        }
    });
    std::jthread t3([] {
        for (int i = 0; i < n; ++i) {
            L::info_raw("{}", i);
        }
    });
    std::jthread t4([] {
        for (int i = 0; i < n; ++i) {
            L::warn_raw("{}", i);
        }
    });
    std::jthread t5([] {
        for (int i = 0; i < n; ++i) {
            L::error_raw("{}", i);
        }
    });
    std::jthread t6([] {
        for (int i = 0; i < n; ++i) {
            L::fatal_raw("{}", i);
        }
    });

    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();
    t6.join();
    L::flush();

    using Lv = huxint::Level;
    return sink->count(Lv::Trace) == n && sink->count(Lv::Debug) == n && sink->count(Lv::Info) == n &&
           sink->count(Lv::Warn) == n && sink->count(Lv::Error) == n && sink->count(Lv::Fatal) == n;
}

// 3. 级别过滤测试
bool test_level_filter() {
    using L = huxint::Logger<"Filter">;
    auto *sink = L::add_sink<huxint::MemorySink>();
    L::level(huxint::Level::Warn);

    L::trace_raw("should not appear");
    L::debug_raw("should not appear");
    L::info_raw("should not appear");
    L::warn_raw("should appear");
    L::error_raw("should appear");
    L::fatal_raw("should appear");
    L::flush();

    return sink->size() == 3;
}

// 4. 带位置信息测试
bool test_with_location() {
    using L = huxint::Logger<"Location">;
    auto *sink = L::add_sink<huxint::MemorySink>();

    L::info("test message");
    L::flush();

    return sink->has_location();
}

// 5. 不带位置信息测试
bool test_without_location() {
    using L = huxint::Logger<"NoLocation">;
    auto *sink = L::add_sink<huxint::MemorySink>();

    L::info_raw("test message");
    L::flush();

    return !sink->has_location();
}

// 6. 命名 Logger 测试
bool test_named_logger() {
    using L = huxint::Logger<"MyLogger">;
    auto *sink = L::add_sink<huxint::MemorySink>();

    L::info_raw("test");
    L::flush();

    return L::name() == "MyLogger" && sink->logs()[0].name == "MyLogger";
}

// 7. 匿名 Logger 测试
bool test_anonymous_logger() {
    using L = huxint::Logger<>;
    auto *sink = L::add_sink<huxint::MemorySink>();

    L::info_raw("test");
    L::flush();

    return L::name().empty() && sink->logs()[0].name.empty();
}

// 8. 多 Sink 测试
bool test_multiple_sinks() {
    using L = huxint::Logger<"MultiSink">;
    auto *sink1 = L::add_sink<huxint::MemorySink>();
    auto *sink2 = L::add_sink<huxint::MemorySink>();

    L::info_raw("test");
    L::flush();

    return sink1->size() == 1 && sink2->size() == 1;
}

// 9. 线程池配置测试
bool test_thread_pool() {
    using L = huxint::Logger<"Pool">;
    auto *sink = L::add_sink<huxint::MemorySink>();
    L::set_thread_count(4);

    for (int i = 0; i < 200; ++i) {
        L::info_raw("log {}", i);
    }
    L::flush();

    return sink->size() == 200;
}

// 10. 格式化参数测试
bool test_format_args() {
    using L = huxint::Logger<"Format">;
    auto *sink = L::add_sink<huxint::MemorySink>();

    L::info_raw("int: {}, str: {}, float: {:.2f}", 42, "hello", 3.14159);
    L::flush();

    return sink->logs()[0].msg == "int: 42, str: hello, float: 3.14";
}

// 11. 压力测试
bool test_stress() {
    using L = huxint::Logger<"Stress">;
    auto *sink = L::add_sink<huxint::MemorySink>();
    L::set_thread_count(8);

    auto start = std::chrono::steady_clock::now();
    run_threads<L>(20, 5000, [](int t, int i) {
        L::info_raw("T{} I{}", t, i);
    });
    L::flush();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();

    std::print("{} logs in {}ms ({} logs/s) ", sink->size(), ms, sink->size() * 1000 / (ms + 1));
    return sink->size() == 100000;
}

// 12. 数据完整性测试
bool test_integrity() {
    using L = huxint::Logger<"Integrity">;
    auto *sink = L::add_sink<huxint::MemorySink>();

    constexpr int n = 1000;
    for (int i = 0; i < n; ++i) {
        L::info_raw("MSG{:04d}", i);
    }
    L::flush();

    if (sink->size() != n) {
        return false;
    }

    for (int i = 0; i < n; ++i) {
        auto expected = std::format("MSG{:04d}", i);
        bool found = std::ranges::any_of(sink->logs(), [&](auto &e) {
            return e.msg == expected;
        });
        if (!found) {
            return false;
        }
    }
    return true;
}

int main() {
    Test tests[] = {
        {"Concurrent logging", test_concurrent},
        {"All log levels", test_levels},
        {"Level filtering", test_level_filter},
        {"With location info", test_with_location},
        {"Without location info", test_without_location},
        {"Named logger", test_named_logger},
        {"Anonymous logger", test_anonymous_logger},
        {"Multiple sinks", test_multiple_sinks},
        {"Thread pool config", test_thread_pool},
        {"Format arguments", test_format_args},
        {"Stress test", test_stress},
        {"Data integrity", test_integrity},
    };

    int passed = 0;
    for (auto &[name, fn] : tests) {
        std::print("[TEST] {:<30} ... ", name);
        if (fn()) {
            std::println("\033[32mPASS\033[0m");
            ++passed;
        } else {
            std::println("\033[31mFAIL\033[0m");
        }
    }

    std::println("\n{}/{} tests passed", passed, std::size(tests));
    return passed == std::size(tests) ? 0 : 1;
}
