#include <huxint/logger.hpp>
#include <chrono>

using namespace huxint;

void test_1() {
    using log = huxint::Logger<>;
    log::add_sink<ConsoleSink<true>>();
    log::add_sink<FileSink>("test.log");
    log::info_raw("number: {} {} {} {}", 1, 2, 3, std::chrono::system_clock::now());
    log::debug("This is a debug message.");
    log::warn("This is a warning message.");
    log::error("This is an error message.");
    log::fatal("This is a fatal error message.");
    log::trace("This is a trace message.");
    log::trace_raw("Raw trace message without location.");
}

void test_2() {
    using app = huxint::Logger<"app">;
    app::add_sink<ConsoleSink<true>>();
    app::info("function: {}", here().function_name());
    app::info("file: {}", here().file_name());
    app::info("line: {}", here().line());
    app::info("number: {} {} {}", 1, 2, 3);
}

int main() {
    test_1();
    test_2();
    return 0;
}