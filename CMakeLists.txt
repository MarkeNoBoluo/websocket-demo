# WebSocket Demo - 根CMake配置
cmake_minimum_required(VERSION 3.16)
project(WebSocketDemo
    VERSION 1.0.0
    DESCRIPTION "Modular WebSocket Client/Server Demo"
    LANGUAGES CXX
)

# 🎯 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 📂 设置输出目录
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 🔧 包含CMake工具脚本
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

# 📦 查找依赖
find_package(Boost 1.86.0 REQUIRED COMPONENTS system thread)

# 🏗️ 按顺序添加模块（依赖顺序很重要！）
add_subdirectory(common)      # 1. 公共基础库
add_subdirectory(core)        # 2. WebSocket核心
add_subdirectory(server)      # 3. 服务器程序
# add_subdirectory(client)      # 4. 客户端库
# add_subdirectory(console_client)  # 5. 控制台客户端

# 💡 构建后信息
message(STATUS "======================================")
message(STATUS "项目配置完成")
message(STATUS "构建类型: ${CMAKE_BUILD_TYPE}")
message(STATUS "输出目录: ${CMAKE_BINARY_DIR}")
message(STATUS "======================================")