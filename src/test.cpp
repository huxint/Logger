#include <huxint/logger.hpp>
#include <string>
#include <thread>
#include <vector>
#include <atomic>
#include <cassert>
#include <print>
#include <regex>

namespace huxint {
    // MemorySink for testing - collects all logs
    class MemorySink final : public Sink {
    public:
        void write(const Level level, std::string_view name, const std::string &msg) override {
            std::scoped_lock lock(mutex_);
            logs_.emplace_back(level, std::string(name), msg);
        }

        void flush() override {}

        struct LogEntry {
            Level level;
            std::string name;
            std::string msg;
            LogEntry(Level l, std::string n, std::string m)
            : level(l),
              name(std::move(n)),
              msg(std::move(m)) {}
        };

        std::vector<LogEntry> get_logs() {
            std::scoped_lock lock(mutex_);
            return logs_;
        }

        void clear() {
            std::scoped_lock lock(mutex_);
            logs_.clear();
        }

        size_t size() {
            std::scoped_lock lock(mutex_);
            return logs_.size();
        }

    private:
        std::vector<LogEntry> logs_;
        std::mutex mutex_;
    };

} // namespace huxint

// Test 1: Concurrent logging from multiple threads
void test_concurrent_logging() {
    std::println("--------------------------------------------------------------------------------------");

    using TestLogger = huxint::Logger<"ConcurrentTest">;
    auto sink = std::make_shared<huxint::MemorySink>();
    TestLogger::add_sink(sink);

    constexpr int num_threads = 10;
    constexpr int logs_per_thread = 100;

    std::vector<std::thread> threads;
    threads.reserve(num_threads);

    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([t]() {
            for (int i = 0; i < logs_per_thread; ++i) {
                TestLogger::info("Thread {} - Log {}", t, i);
            }
        });
    }

    for (auto &th : threads) {
        th.join();
    }

    TestLogger::flush();

    auto logs = sink->get_logs();
    std::println("Expected: {}", num_threads * logs_per_thread);
    std::println("Actual:   {}", logs.size());

    assert(logs.size() == num_threads * logs_per_thread);
    std::println("[PASS] All logs recorded correctly");
}

// Test 2: Log content integrity (no data race corruption)
void test_log_integrity() {
    std::println("--------------------------------------------------------------------------------------");

    using IntegrityLogger = huxint::Logger<"IntegrityTest">;
    auto sink = std::make_shared<huxint::MemorySink>();
    IntegrityLogger::add_sink(sink);

    constexpr int num_threads = 5;
    constexpr int logs_per_thread = 50;

    std::vector<std::thread> threads;
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([t]() {
            for (int i = 0; i < logs_per_thread; ++i) {
                IntegrityLogger::info("T{:02d}I{:03d}", t, i);
            }
        });
    }

    for (auto &th : threads) {
        th.join();
    }

    IntegrityLogger::flush();

    auto logs = sink->get_logs();
    std::regex pattern(R"(T\d{2}I\d{3})");
    int valid_count = 0;

    for (const auto &log : logs) {
        if (std::regex_match(log.msg, pattern)) {
            ++valid_count;
        } else {
            std::println("[FAIL] Corrupted log: {}", log.msg);
        }
    }

    std::println("Valid logs: {}/{}", valid_count, logs.size());
    assert(valid_count == static_cast<int>(logs.size()));
    std::println("[PASS] All log contents intact");
}

// Test 3: Thread pool configuration
void test_thread_pool_config() {
    std::println("--------------------------------------------------------------------------------------");

    using PoolLogger = huxint::Logger<"PoolTest">;
    auto sink = std::make_shared<huxint::MemorySink>();
    PoolLogger::add_sink(sink);
    PoolLogger::set_thread_count(4);

    constexpr int total_logs = 200;

    for (int i = 0; i < total_logs; ++i) {
        PoolLogger::info("Log entry {}", i);
    }

    PoolLogger::flush();

    auto logs = sink->get_logs();
    std::println("Pool size: 4");
    std::println("Expected: {}", total_logs);
    std::println("Actual:   {}", logs.size());

    assert(logs.size() == total_logs);
    std::println("[PASS] Thread pool processed all logs");
}

// Test 4: Concurrent logging at different levels
void test_concurrent_levels() {
    std::println("--------------------------------------------------------------------------------------");
    using LevelLogger = huxint::Logger<"LevelTest">;
    auto sink = std::make_shared<huxint::MemorySink>();
    LevelLogger::add_sink(sink);

    std::vector<std::thread> threads;
    constexpr int logs_per_level = 50;

    threads.emplace_back([]() {
        for (int i = 0; i < logs_per_level; ++i) {
            LevelLogger::trace("trace {}", i);
        }
    });
    threads.emplace_back([]() {
        for (int i = 0; i < logs_per_level; ++i) {
            LevelLogger::debug("debug {}", i);
        }
    });
    threads.emplace_back([]() {
        for (int i = 0; i < logs_per_level; ++i) {
            LevelLogger::info("info {}", i);
        }
    });
    threads.emplace_back([]() {
        for (int i = 0; i < logs_per_level; ++i) {
            LevelLogger::warn("warn {}", i);
        }
    });
    threads.emplace_back([]() {
        for (int i = 0; i < logs_per_level; ++i) {
            LevelLogger::error("error {}", i);
        }
    });
    threads.emplace_back([]() {
        for (int i = 0; i < logs_per_level; ++i) {
            LevelLogger::fatal("fatal {}", i);
        }
    });

    for (auto &th : threads) {
        th.join();
    }

    LevelLogger::flush();

    auto logs = sink->get_logs();

    int counts[6] = {0};
    for (const auto &log : logs) {
        counts[static_cast<int>(log.level)]++;
    }

    std::println("TRACE: {}, DEBUG: {}, INFO: {}, WARN: {}, ERROR: {}, FATAL: {}",
                 counts[0],
                 counts[1],
                 counts[2],
                 counts[3],
                 counts[4],
                 counts[5]);

    for (int i = 0; i < 6; ++i) {
        assert(counts[i] == logs_per_level);
    }
    std::println("[PASS] All log levels recorded correctly");
}

// Test 5: High concurrency stress test
void test_stress() {
    std::println("--------------------------------------------------------------------------------------");

    using StressLogger = huxint::Logger<"StressTest">;
    auto sink = std::make_shared<huxint::MemorySink>();
    StressLogger::add_sink(sink);
    StressLogger::set_thread_count(8);

    constexpr int num_threads = 20;
    constexpr int logs_per_thread = 5000;
    std::atomic<int> submitted{0};

    auto start = std::chrono::high_resolution_clock::now();

    std::vector<std::thread> threads;
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([t, &submitted]() {
            for (int i = 0; i < logs_per_thread; ++i) {
                StressLogger::info("Stress T{} I{}", t, i);
                submitted.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }

    for (auto &th : threads) {
        th.join();
    }

    StressLogger::flush();

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    auto logs = sink->get_logs();
    std::println("Submitted: {}", submitted.load());
    std::println("Recorded:  {}", logs.size());
    std::println("Duration:  {}ms", duration.count());
    std::println("Throughput: {} logs/s", logs.size() * 1000 / (duration.count() + 1));

    assert(logs.size() == num_threads * logs_per_thread);
    std::println("[PASS] All logs recorded under high concurrency");
}

int main() {
    test_concurrent_logging();
    test_log_integrity();
    test_thread_pool_config();
    test_concurrent_levels();
    test_stress();
    std::println("All tests passed!");
    return 0;
}
