# websocket-demo

这是一个示例项目，演示使用 websocketpp + Boost 构建一个简单的 WebSocket echo server。

目录结构
- server/
  - CMakeLists.txt
  - include/
    - global.h
  - src/
    - main.cpp
- CMakeLists.txt (顶层)

快速开始（本地构建）
1. 准备依赖
   - Boost（1.86 推荐），若在 Windows 上可使用预编译二进制或 vcpkg。请确保包含目录与库目录可被 CMake 找到，或使用 -DBoost_ROOT=... 指定。
   - websocketpp（头文件即可）。可在系统 include 路径中，或通过 -DWEBSOCKETPP_ROOT=... 指定路径。

2. 构建
```bash
git clone <repo-url>
cd websocket-demo
mkdir build
cd build
cmake .. -DBoost_ROOT="/path/to/boost" -DWEBSOCKETPP_ROOT="/path/to/websocketpp"
cmake --build . --config Release
```

3. 运行
- 可执行文件在 build/bin 或 build/<config>/bin（取决于平台/配置）。
- Windows 下如果使用动态 Boost，请确保对应的 Boost DLL 在 PATH 中或放在可执行文件目录。

常见问题
- 无法找到 Boost：使用 -DBoost_ROOT 或设置环境变量 BOOST_ROOT。
- websocketpp 仅是头文件库，确保 WEBSOCKETPP_ROOT 指向其 include 目录（包含 websocketpp/config/...）。

更多：建议使用包管理（vcpkg/conan）将 Boost 与 websocketpp 编入项目，减少平台差异。
``` ````

构建与运行示例（命令总结）
- 在 Linux/macOS（系统安装 Boost & websocketpp）：
  - mkdir build && cd build
  - cmake .. 
  - cmake --build .
  - ./bin/WebSocketServer
- 在 Windows（手动 Boost 路径）：
  - cmake .. -DBoost_ROOT="D:/LIB/Boost/boost_1_86_0" -DWEBSOCKETPP_ROOT="D:/LIB/websocketpp"
  - cmake --build . --config Release
  - 运行可执行（注意复制 boost_system/boost_thread 的 DLL，或使用静态链接）

额外建议（可选）
- 使用 vcpkg 并通过 CMakeToolchain 文件集成：vcpkg install boost-asio boost-system boost-thread websocketpp（可减少平台差异）。
- 把 websocketpp 作为 Git 子模块或使用 FetchContent 自动下载（如果你希望 CI 无需外部依赖）。
- 写一个简单的 unit test target（gtest）用于验证 echo 行为。

如果你希望，我可以：
- 直接在��的仓库里创建这些文件并打开一个 PR（需要 repo owner/路径信息与权限），或者
- 把上述文件内容打包为一个 patch/zip 发给你，或
- 把 CMakeLists 做得更智能（增加 FetchContent 自动拉取 websocketpp、支持 vcpkg 变量、自动复制 DLL 列表等）。 

告诉我你想要我接下来做哪一步：直接在仓库里提 PR（请确认仓库权限/owner/name），还是我把完整文件内容贴出来便于你本地创建。
