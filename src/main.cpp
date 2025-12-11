#include <huxint/logger.hpp>
#include <chrono>
#include <source_location>

int main() {
    using namespace huxint;
    using Log = huxint::Logger<>;
    Log::add_sink<ConsoleSink<true>>();
    Log::add_sink<FileSink>("log.txt");

    Log::info("number: {} {} {} {}", 1, 2, 3, std::chrono::system_clock::now());
    Log::warn("This is a warning message.");
    Log::error("This is an error message.");
    Log::fatal("This is a fatal error message.");
    Log::trace("This is a trace message.");

    using App = huxint::Logger<"App">;
    App::add_sink<ConsoleSink<true>>();

    App::info("number: {} {} {}", 1, 2, 3);
    App::warn("This is a warning message.");
    App::error("This is an error message.");
    App::fatal("This is a fatal error message.");
    App::trace("This is a trace message.");

    using huxint = huxint::Logger<"huxint">;
    huxint::add_sink<ConsoleSink<true>>();
    huxint::add_sink<FileSink>("huxint.log");
    huxint::info(
        "file {}, line {}", std::source_location::current().file_name(), std::source_location::current().line());

    huxint::info("function: {}", std::source_location::current().function_name());
    return 0;
}