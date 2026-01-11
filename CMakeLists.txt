# WebSocket Demo - 根CMake配置
cmake_minimum_required(VERSION 3.16)

# 🔧 设置CMake策略以避免警告
if(POLICY CMP0144)
    cmake_policy(SET CMP0144 NEW)  # 使用<PACKAGENAME>_ROOT变量
endif()

if(POLICY CMP0167)
    cmake_policy(SET CMP0167 OLD)  # 继续使用FindBoost模块
endif()

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

# ========== 强制指定Boost路径 ==========
# 清除可能的CMake缓存干扰
unset(Boost_FOUND CACHE)
unset(Boost_INCLUDE_DIR CACHE)
unset(Boost_LIBRARY_DIRS CACHE)

set(Boost_ROOT "D:/LIB/Boost/boost_1_86_0")
set(BOOST_ROOT "D:/LIB/Boost/boost_1_86_0")
set(Boost_INCLUDE_DIR "D:/LIB/Boost/boost_1_86_0")
set(Boost_LIBRARY_DIR "D:/LIB/Boost/boost_1_86_0/stage/lib")
set(BOOST_LIBRARYDIR "D:/LIB/Boost/boost_1_86_0/stage/lib")
set(Boost_NO_SYSTEM_PATHS ON)
set(Boost_NO_BOOST_CMAKE ON)

# WebSocket++路径
set(WEBSOCKETPP_ROOT "D:/LIB/websocketpp")

# 配置Boost查找选项
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
set(Boost_DETAILED_FAILURE_MSG ON)

find_package(Boost 1.86.0 REQUIRED
    COMPONENTS
        system
        thread
        chrono
        date_time
        random
    OPTIONAL_COMPONENTS
        context
        filesystem
)

if(Boost_FOUND)
    message(STATUS "✓ Boost ${Boost_VERSION} 找到")
    message(STATUS "  包含目录: ${Boost_INCLUDE_DIRS}")
    message(STATUS "  库目录: ${Boost_LIBRARY_DIRS}")
else()
    message(FATAL_ERROR "✗ 未找到 Boost 1.86.0")
endif()

# 🏗️ 按顺序添加模块（依赖顺序很重要！）
add_subdirectory(common)      # 1. 公共基础库
add_subdirectory(core)        # 2. WebSocket核心
add_subdirectory(server)      # 3. 服务器程序
add_subdirectory(client)      # 4. 客户端库
add_subdirectory(console_client)  # 5. 控制台客户端

# Windows平台DLL复制
if(WIN32 AND NOT Boost_USE_STATIC_LIBS)
    add_custom_target(copy_boost_dlls ALL
        COMMAND ${CMAKE_COMMAND} -E echo "复制Boost DLL..."
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${Boost_LIBRARY_DIR}/boost_system-vc143-mt-x64-1_86.dll"
            "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${Boost_LIBRARY_DIR}/boost_thread-vc143-mt-x64-1_86.dll"
            "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
        COMMENT "复制Boost DLL到输出目录"
    )
endif()

# 💡 构建后信息
message(STATUS "======================================")
message(STATUS "项目配置完成")
message(STATUS "构建类型: ${CMAKE_BUILD_TYPE}")
message(STATUS "输出目录: ${CMAKE_BINARY_DIR}")
message(STATUS "======================================")
