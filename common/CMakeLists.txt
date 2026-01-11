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

# 设置编译选项
if(MSVC)
    target_compile_options(ws-common PRIVATE /W4)
else()
    target_compile_options(ws-common PRIVATE -Wall -Wextra)
endif()

message(STATUS "公共库配置完成: ws-common")