# WebSocket核心库配置
project(ws-core LANGUAGES CXX)

# 查找WebSocket++
find_path(WEBSOCKETPP_INCLUDE_DIR
    NAMES websocketpp/version.hpp
    HINTS
        ${CMAKE_SOURCE_DIR}/third_party/websocketpp
        /usr/local/include
        /usr/include
)

if(NOT WEBSOCKETPP_INCLUDE_DIR)
    message(WARNING "WebSocket++ not found, please install or add to third_party/")
endif()

# 创建静态库（更简单，避免 DLL 导出问题）
add_library(ws-core STATIC
    src/connection.cpp
)

# 包含目录
target_include_directories(ws-core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        ${WEBSOCKETPP_INCLUDE_DIR}
        ${Boost_INCLUDE_DIRS}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# 链接依赖
target_link_libraries(ws-core
    PUBLIC
        ws-common
        Boost::system
        Boost::thread
)

# 平台特定库
if(WIN32)
    target_link_libraries(ws-core PRIVATE ws2_32)
endif()

message(STATUS "✓ 核心库配置完成: ws-core (静态库)")
