# WebSocket服务器程序配置
project(WebSocketServer LANGUAGES CXX)

# 创建可执行文件
add_executable(WebSocketServer
    src/main.cpp
    src/server.cpp
)

# 包含目录
target_include_directories(WebSocketServer
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${WEBSOCKETPP_ROOT}
        ${Boost_INCLUDE_DIRS}
)

# 链接库
target_link_libraries(WebSocketServer
    PRIVATE
        ws-core
        ws-common
        Boost::system
        Boost::thread
)

# Windows平台特定设置
if(WIN32)
    target_compile_definitions(WebSocketServer PRIVATE
        _WIN32_WINNT=0x0601
        _WEBSOCKETPP_CPP11_STL_
        NOMINMAX
    )
    
    target_link_libraries(WebSocketServer PRIVATE
        ws2_32
        wsock32
    )
endif()

# 设置编译选项
if(MSVC)
    target_compile_options(WebSocketServer PRIVATE /W4)
else()
    target_compile_options(WebSocketServer PRIVATE -Wall -Wextra)
endif()

message(STATUS "✓ 服务器程序配置完成: WebSocketServer")