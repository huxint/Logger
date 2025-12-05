#include "huxint/logger/sink.hpp"
#include <huxint/logger.hpp>

int main() {
    using namespace huxint;
    using Log = huxint::Logger<>;
    Log::add_sink(std::make_shared<ConsoleSink>(true));
    Log::add_sink(std::make_shared<FileSink>("log.txt"));

    Log::info("number: {} {} {} {:%Y-%m-%d %H:%M:%S}",
              1,
              2,
              3,
              std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now()));
    Log::warn("This is a warning message.");
    Log::error("This is an error message.");
    Log::fatal("This is a fatal error message.");
    Log::trace("This is a trace message.");

    using App = huxint::Logger<"App">;
    App::add_sink(std::make_shared<ConsoleSink>(true));

    App::info("number: {} {} {}", 1, 2, 3);
    App::warn("This is a warning message.");
    App::error("This is an error message.");
    App::fatal("This is a fatal error message.");
    App::trace("This is a trace message.");

    return 0;
}