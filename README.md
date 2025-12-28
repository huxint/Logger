# Huxint Logger

一个现代 C++23 异步日志库，支持多线程、多输出目标和彩色控制台输出。

## 特性

- 异步日志写入（基于线程池）
- 6 个日志级别：Trace、Debug、Info、Warn、Error、Fatal
- 支持多个输出目标（Sink）
- 控制台彩色输出（支持 Windows ANSI）
- 文件输出（带时间戳）
- 编译期 Logger 命名
- 类型安全的 `std::format` 格式化

## 要求

- C++23 编译器
- CMake 4.0+

## 构建

```bash
cmake -B build
cmake --build build
```

## 使用示例

```cpp
#include <huxint/logger.hpp>

int main() {
    using namespace huxint;
    
    using log = Logger<>;
    // 添加两个输出目标
    log::add_sink<ConsoleSink<true>>();
    log::add_sink<FileSink>("app.log");
    
    // 设置日志级别
    log::set_level(Level::Debug);
    
    log::info("Hello, {}!", "World");
    log::warn("Warning message");
    log::error("Error code: {}", 42);

    // 使用命名 Logger
    using app = Logger<"app">;
    app::add_sink<ConsoleSink<true>>();
    app::set_thread_count(4);
    app::info("Module specific log");
    app::info("line: {}", here().line());
    app::info("file: {}", here().file_name());
    app::info("function: {}", here().function_name());
    return 0;
}
```

## 日志级别

| 级别 | 颜色 | 用途 |
|------|------|------|
| Trace | 灰色 | 详细追踪信息 |
| Debug | 青色 | 调试信息 |
| Info | 绿色 | 一般信息 |
| Warn | 黄色 | 警告信息 |
| Error | 红色 | 错误信息 |
| Fatal | 紫色 | 致命错误 |

## 线程池

本项目使用自研的 C++23 线程池，详见 [ThreadPool](https://github.com/huxint/ThreadPool)