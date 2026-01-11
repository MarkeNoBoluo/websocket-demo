# WebSocket核心库配置
project(ws-core LANGUAGES CXX)

# 查找WebSocket++
find_package(Websocketpp REQUIRED)

if(NOT WEBSOCKETPP_INCLUDE_DIR)
    message(FATAL_ERROR "✗ WebSocket++ 未找到！请确保 WEBSOCKETPP_ROOT (${WEBSOCKETPP_ROOT}) 指向正确的目录")
endif()

# 创建共享库
add_library(ws-core SHARED
    src/connection.cpp
)

# 包含目录
target_include_directories(ws-core
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/ws_core>
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
        Websocketpp::Websocketpp
)

# 平台特定库
if(WIN32)
    target_link_libraries(ws-core PRIVATE ws2_32)
endif()

message(STATUS "核心库配置完成: ws-core")