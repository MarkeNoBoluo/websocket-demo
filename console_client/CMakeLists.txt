# 控制台客户端程序配置
project(ConsoleClient LANGUAGES CXX)

# 创建可执行文件
add_executable(ConsoleClient
    src/main.cpp
)

# 包含目录
target_include_directories(ConsoleClient
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/client/include  # 客户端SDK头文件
)

# 链接库
target_link_libraries(ConsoleClient
    PRIVATE
        ws-client    # 客户端SDK库
        ws-core      # WebSocket核心库
        ws-common    # 公共基础库
)

# Windows平台特定设置
if(WIN32)
    target_link_libraries(ConsoleClient PRIVATE ws2_32)
endif()

# 设置编译选项
if(MSVC)
    target_compile_options(ConsoleClient PRIVATE /W4)
else()
    target_compile_options(ConsoleClient PRIVATE -Wall -Wextra)
endif()

# 安装配置
install(TARGETS ConsoleClient
    RUNTIME DESTINATION bin
)

message(STATUS "✓ 控制台客户端配置完成: ConsoleClient")
