#include "ws_server/server.hpp"
#include "ws_common/logger.hpp"
#include <iostream>
#include <csignal>
#include <atomic>
#include <thread>

// 全局服务器指针，用于信号处理
std::atomic<bool> running{true};
KK_WS::server::WebSocketServer* g_server = nullptr;

// 信号处理函数
void signal_handler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        KK_WS::Logger::info("收到停止信号，正在关闭服务器...");
        running = false;
        
        if (g_server) {
            g_server->stop();
        }
    }
}

int main(int argc, char* argv[]) {
    // 设置日志级别
    KK_WS::Logger::set_level(KK_WS::Logger::Level::Ws_INFO);
    
    KK_WS::Logger::info("===========================================");
    KK_WS::Logger::info("    WebSocket Echo Server v1.0");
    KK_WS::Logger::info("===========================================");

    // 解析命令行参数
    KK_WS::server::ServerConfig config;
    
    if (argc > 1) {
        try {
            config.port = static_cast<uint16_t>(std::stoi(argv[1]));
        } catch (...) {
            KK_WS::Logger::warning("无效的端口号，使用默认端口 9002");
        }
    }

    try {
        // 创建服务器
        KK_WS::server::WebSocketServer server(config);
        g_server = &server;

        // 注册信号处理器
        std::signal(SIGINT, signal_handler);
        std::signal(SIGTERM, signal_handler);

        // 设置消息处理器（Echo模式）
        server.set_message_handler([&server](auto hdl, const std::string& message) {
            KK_WS::Logger::info("收到消息: " + message);
            
            // 回显消息给发送者
            server.send_message(hdl, "Echo: " + message);
            
            // 也可以选择广播给所有客户端
            // server.broadcast("Broadcast: " + message);
        });

        // 设置连接处理器
        server.set_open_handler([&server](auto hdl) {
            server.send_message(hdl, "欢迎连接到WebSocket Echo Server!");
            KK_WS::Logger::info("当前连接数: " + std::to_string(server.get_connection_count()));
        });

        server.set_close_handler([&server](auto hdl) {
            KK_WS::Logger::info("当前连接数: " + std::to_string(server.get_connection_count()));
        });

        KK_WS::Logger::info("");
        KK_WS::Logger::info("使用说明:");
        KK_WS::Logger::info("  - 服务器将回显收到的所有消息");
        KK_WS::Logger::info("  - 按 Ctrl+C 停止服务器");
        KK_WS::Logger::info("");
        KK_WS::Logger::info("测试命令 (使用 websocat 或其他客户端):");
        KK_WS::Logger::info("  websocat ws://localhost:" + std::to_string(config.port));
        KK_WS::Logger::info("");

        // 启动服务器（阻塞）
        server.start();

        g_server = nullptr;
        KK_WS::Logger::info("服务器已正常退出");

    } catch (const std::exception& e) {
        KK_WS::Logger::error("服务器错误: " + std::string(e.what()));
        return 1;
    }

    return 0;
}
