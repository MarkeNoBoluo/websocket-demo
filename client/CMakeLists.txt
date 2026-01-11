# 客户端SDK库配置
project(ws-client-lib LANGUAGES CXX)

# 创建静态库（便于使用）
add_library(ws-client STATIC
    src/client.cpp
)

# 包含目录
target_include_directories(ws-client
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# 链接依赖
target_link_libraries(ws-client
    PUBLIC
        ws-core      # WebSocket核心库
        ws-common    # 公共基础库
)

# 设置编译选项
if(MSVC)
    target_compile_options(ws-client PRIVATE /W4)
else()
    target_compile_options(ws-client PRIVATE -Wall -Wextra)
endif()

# 安装配置
install(TARGETS ws-client
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/ws_client
    DESTINATION include
    FILES_MATCHING PATTERN "*.hpp"
)

message(STATUS "✓ 客户端SDK配置完成: ws-client")
