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
mkdir build && cd build
cmake ..
cmake --build .
```

## 使用示例

```cpp
#include <huxint/logger.hpp>

int main() {
    using namespace huxint;
    
    // 添加控制台输出（带颜色）
    Logger<>::add_sink(std::make_shared<ConsoleSink>(true));
    
    // 添加文件输出
    Logger<>::add_sink(std::make_shared<FileSink>("app.log"));
    
    // 设置日志级别
    Logger<>::set_level(Level::Debug);
    
    // 输出日志
    Logger<>::info("Hello, {}!", "World");
    Logger<>::warn("Warning message");
    Logger<>::error("Error code: {}", 42);

    using App = Logger<"MyModule">;
    
    // 使用命名 Logger
    App::info("Module specific log");
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