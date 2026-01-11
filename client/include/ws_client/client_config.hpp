#pragma once
#include <string>
#include <cstdint>

namespace KK_WS::client {

/**
 * @brief 客户端配置结构
 */
struct ClientConfig {
    std::string server_uri = "ws://localhost:9002";  // 服务器地址
    uint16_t server_port = 9002;                     // 服务器端口
    bool auto_reconnect = true;                      // 自动重连
    uint32_t reconnect_interval_ms = 3000;           // 重连间隔
    uint32_t ping_interval_ms = 10000;               // 心跳间隔
    uint32_t connect_timeout_ms = 5000;              // 连接超时
    bool verbose_logging = false;                    // 详细日志

    // 构建完整的WebSocket URI
    std::string get_full_uri() const {
        if (!server_uri.empty()) {
            return server_uri;
        }
        return "ws://" + std::string("localhost") + ":" + std::to_string(server_port);
    }

    // 验证配置
    bool validate() const {
        return !get_full_uri().empty();
    }
};

} // namespace ws::client
