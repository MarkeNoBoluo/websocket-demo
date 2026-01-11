# 公共库配置
project(ws-common LANGUAGES CXX)

# 创建静态库
add_library(ws-common STATIC
    src/logger.cpp
)

# 包含目录
target_include_directories(ws-common
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# 链接依赖（当前无外部依赖）
# target_link_libraries(ws-common PUBLIC ...)

message(STATUS "公共库配置完成: ws-common")