cmake_minimum_required(VERSION 4.0)

project(Logger LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(Logger src/main.cpp)
target_link_libraries(Logger stdc++exp)
target_include_directories(Logger PRIVATE include external/thread-pool/include)

# 多线程日志测试
add_executable(Test src/test.cpp)
target_link_libraries(Test stdc++exp)
target_include_directories(Test PRIVATE include external/thread-pool/include)