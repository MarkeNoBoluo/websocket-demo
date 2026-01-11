#include "ws_client/client.hpp"
#include "ws_common/logger.hpp"
#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <csignal>

using namespace KK_WS::client;

// 全局标志
std::atomic<bool> running{true};
std::shared_ptr<WebSocketClient> g_client = nullptr;

// 信号处理
void signal_handler(int signal) {
    if (signal == SIGINT) {
        std::cout << "\n收到停止信号，正在退出..." << std::endl;
        running = false;

        if (g_client) {
            g_client->disconnect();
        }
    }
}

// 打印帮助信息
void print_help() {
    std::cout << "\n🎯 WebSocket控制台客户端 - 命令列表\n";
    std::cout << "========================================\n";
    std::cout << "  send <消息>    - 发送文本消息\n";
    std::cout << "  binary <数据>  - 发送二进制数据\n";
    std::cout << "  sub <主题>     - 订阅主题\n";
    std::cout << "  unsub <主题>   - 取消订阅\n";
    std::cout << "  list           - 显示订阅列表\n";
    std::cout << "  stats          - 显示统计信息\n";
    std::cout << "  status         - 显示连接状态\n";
    std::cout << "  disconnect     - 断开连接\n";
    std::cout << "  reconnect      - 重新连接\n";
    std::cout << "  help           - 显示此帮助\n";
    std::cout << "  quit           - 退出程序\n";
    std::cout << "========================================\n";
}

// 解析命令
void process_command(const std::string& command, WebSocketClient& client) {
    if (command.empty()) return;

    // 查找空格分隔命令和参数
    size_t space_pos = command.find(' ');
    std::string cmd = command;
    std::string args;

    if (space_pos != std::string::npos) {
        cmd = command.substr(0, space_pos);
        args = command.substr(space_pos + 1);
    }

    // 转换为小写
    // std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);

    try {
        if (cmd == "send") {
            if (!args.empty()) {
                client.send_text(args);
                std::cout << "✅ 消息已发送\n";
            } else {
                std::cout << "❌ 用法: send <消息内容>\n";
            }
        }
        else if (cmd == "binary") {
            if (!args.empty()) {
                client.send_binary(args);
                std::cout << "✅ 二进制数据已发送\n";
            } else {
                std::cout << "❌ 用法: binary <数据>\n";
            }
        }
        else if (cmd == "sub") {
            if (!args.empty()) {
                client.subscribe(args);
                std::cout << "✅ 已订阅主题: " << args << "\n";
            } else {
                std::cout << "❌ 用法: sub <主题名称>\n";
            }
        }
        else if (cmd == "unsub") {
            if (!args.empty()) {
                client.unsubscribe(args);
                std::cout << "✅ 已取消订阅: " << args << "\n";
            } else {
                std::cout << "❌ 用法: unsub <主题名称>\n";
            }
        }
        else if (cmd == "list") {
            auto subs = client.get_subscriptions();
            if (subs.empty()) {
                std::cout << "📭 没有订阅任何主题\n";
            } else {
                std::cout << "📋 订阅的主题 (" << subs.size() << "):\n";
                for (const auto& topic : subs) {
                    std::cout << "  - " << topic << "\n";
                }
            }
        }
        else if (cmd == "stats") {
            std::cout << "📊 客户端统计信息:\n";
            std::cout << "  客户端ID: " << client.get_client_id() << "\n";
            std::cout << "  服务器: " << client.get_server_uri() << "\n";
            std::cout << "  连接时长: " << client.get_connection_duration() / 1000 << "秒\n";
            std::cout << "  发送消息: " << client.get_messages_sent() << "\n";
            std::cout << "  接收消息: " << client.get_messages_received() << "\n";
        }
        else if (cmd == "status") {
            auto state = client.get_connection_state();
            std::cout << "📡 连接状态: ";
            switch (state) {
            case KK_WS::ws_connection_state::WS_DISCONNECTED: std::cout << "断开连接\n"; break;
            case KK_WS::ws_connection_state::WS_CONNECTING: std::cout << "连接中...\n"; break;
            case KK_WS::ws_connection_state::WS_CONNECTED: std::cout << "已连接 ✓\n"; break;
            case KK_WS::ws_connection_state::WS_DISCONNICTING: std::cout << "断开中...\n"; break;
            case KK_WS::ws_connection_state::WS_FAILED: std::cout << "连接失败 ✗\n"; break;
            }
        }
        else if (cmd == "disconnect") {
            client.disconnect();
            std::cout << "🔌 已断开连接\n";
        }
        else if (cmd == "reconnect") {
            client.disconnect();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            // 这里需要重新配置连接
            std::cout << "⚠️  请重新设置连接配置\n";
        }
        else if (cmd == "help") {
            print_help();
        }
        else if (cmd == "quit") {
            running = false;
            std::cout << "👋 正在退出...\n";
        }
        else {
            std::cout << "❓ 未知命令，输入 'help' 查看帮助\n";
        }
    }
    catch (const std::exception& e) {
        std::cout << "💥 执行命令时出错: " << e.what() << "\n";
    }
}

int main(int argc, char* argv[]) {
    // 设置日志级别
    KK_WS::Logger::set_level(KK_WS::Logger::Level::Ws_INFO);

    std::cout << "========================================\n";
    std::cout << "    WebSocket控制台客户端 v1.0\n";
    std::cout << "========================================\n\n";

    // 注册信号处理
    std::signal(SIGINT, signal_handler);

    try {
        // 创建客户端
        g_client = std::make_shared<WebSocketClient>("console_client");

        // 设置消息回调
        g_client->set_message_callback([](const KK_WS::ws_message& msg) {
            std::cout << "\n📨 收到消息: ";

            if (msg.type == KK_WS::ws_message::message_type::TEXT) {
                std::cout << msg.payload;
            } else if (msg.type == KK_WS::ws_message::message_type::BINARY) {
                std::cout << "[二进制 " << msg.payload.size() << " 字节]";
            } else {
                std::cout << "[其他类型消息]";
            }

            std::cout << "\n> " << std::flush;
        });

        // 设置状态回调
        g_client->set_state_callback([](KK_WS::ws_connection_state state) {
            std::cout << "\n⚡ 状态变更: ";

            switch (state) {
            case KK_WS::ws_connection_state::WS_CONNECTED:
                std::cout << "✅ 连接成功!\n"; break;
            case KK_WS::ws_connection_state::WS_DISCONNECTED:
                std::cout << "🔌 连接断开\n"; break;
            case KK_WS::ws_connection_state::WS_FAILED:
                std::cout << "💥 连接失败\n"; break;
            default:
                break;
            }

            std::cout << "> " << std::flush;
        });

        // 设置错误回调
        g_client->set_error_callback([](const std::string& error) {
            std::cout << "\n💥 错误: " << error << "\n";
            std::cout << "> " << std::flush;
        });

        // 配置连接
        ClientConfig config;

        // 解析命令行参数
        if (argc > 1) {
            std::string arg = argv[1];
            if (arg.find("ws://") == 0 || arg.find("wss://") == 0) {
                config.server_uri = arg;
            } else {
                try {
                    config.server_port = static_cast<uint16_t>(std::stoi(arg));
                } catch (...) {
                    std::cout << "⚠️  使用默认端口 9002\n";
                }
            }
        }

        if (argc > 2) {
            try {
                config.server_port = static_cast<uint16_t>(std::stoi(argv[2]));
            } catch (...) {
                // 忽略无效端口
            }
        }

        // 尝试连接
        std::cout << "🔄 正在连接到: " << config.get_full_uri() << "\n";

        if (!g_client->connect(config)) {
            std::cout << "💥 连接失败!\n";
            return 1;
        }

        // 等待连接建立
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        // 显示帮助
        print_help();
        std::cout << "\n💡 提示: 输入 'help' 查看命令列表\n";
        std::cout << "👉 输入命令:\n";

        // 主循环
        std::string input;
        while (running && g_client->get_connection_state() != KK_WS::ws_connection_state::WS_FAILED) {
            std::cout << "> ";

            if (!std::getline(std::cin, input)) {
                break; // EOF
            }

            process_command(input, *g_client);

            if (!running) {
                break;
            }
        }

        // 清理
        if (g_client) {
            g_client->disconnect();
        }

        std::cout << "\n👋 程序退出\n";

    } catch (const std::exception& e) {
        std::cerr << "💥 程序异常: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
