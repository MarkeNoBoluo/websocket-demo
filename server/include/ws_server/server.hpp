#pragma once

#include "ws_common/interface.hpp"
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <memory>
#include <set>
#include <functional>

namespace KK_WS::server {

using server_t = websocketpp::server<websocketpp::config::asio>;
using connection_hdl = websocketpp::connection_hdl;
using message_ptr = server_t::message_ptr;

/**
 * @brief WebSocket服务器配置
 */
struct ServerConfig {
    uint16_t port = 9002;
    bool enable_logging = true;
    std::string bind_address = "0.0.0.0";
};

/**
 * @brief WebSocket服务器类
 * 
 * 提供简单的WebSocket服务器功能，支持多客户端连接
 */
class WebSocketServer {
public:
    using MessageHandler = std::function<void(connection_hdl, const std::string&)>;
    using ConnectionHandler = std::function<void(connection_hdl)>;

    explicit WebSocketServer(const ServerConfig& config = ServerConfig{});
    ~WebSocketServer();

    // 禁止拷贝和移动
    WebSocketServer(const WebSocketServer&) = delete;
    WebSocketServer& operator=(const WebSocketServer&) = delete;

    /**
     * @brief 启动服务器
     */
    void start();

    /**
     * @brief 停止服务器
     */
    void stop();

    /**
     * @brief 向指定客户端发送消息
     */
    void send_message(connection_hdl hdl, const std::string& message);

    /**
     * @brief 向所有客户端广播消息
     */
    void broadcast(const std::string& message);

    /**
     * @brief 设置消息处理回调
     */
    void set_message_handler(MessageHandler handler);

    /**
     * @brief 设置连接建立回调
     */
    void set_open_handler(ConnectionHandler handler);

    /**
     * @brief 设置连接关闭回调
     */
    void set_close_handler(ConnectionHandler handler);

    /**
     * @brief 获取当前连接数
     */
    size_t get_connection_count() const;

private:
    void on_open(connection_hdl hdl);
    void on_close(connection_hdl hdl);
    void on_message(connection_hdl hdl, message_ptr msg);

private:
    ServerConfig config_;
    server_t endpoint_;
    std::set<connection_hdl, std::owner_less<connection_hdl>> connections_;
    
    MessageHandler message_handler_;
    ConnectionHandler open_handler_;
    ConnectionHandler close_handler_;
    
    mutable std::mutex connections_mutex_;
};

} // namespace KK_WS::server
