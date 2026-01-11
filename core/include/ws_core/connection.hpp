#pragma once
#include "ws_common/interface.hpp"
#include <memory>
#include <string>

namespace ws::core {

/**
 * @brief WebSocket连接实现类（Pimpl模式）
 * 
 * 这个类封装了WebSocket++的底层实现，提供了统一的接口
 */
class Connection : public IWebSocketEndpoint {
public:
    Connection();
    ~Connection() override;

    // 禁止拷贝和移动
    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;
    Connection(Connection&&) = delete;
    Connection& operator=(Connection&&) = delete;

    // IWebSocketEndpoint接口实现
    void connect() override;
    void disconnect() override;
    ws_connection_state get_connection_state() const override;
    void send_message(const ws_message& message) override;
    void set_message_callback(MessageCallback callback) override;
    void set_state_callback(StateCallback callback) override;
    void set_error_callback(ErrorCallback callback) override;

    // 配置方法
    void set_config(const ws_config& config);
    const ws_config& get_config() const;

private:
    class ConnectionImpl;
    std::unique_ptr<ConnectionImpl> impl_;
};

/**
 * @brief 创建WebSocket连接的工厂函数
 */
std::unique_ptr<IWebSocketEndpoint> create_connection(const ws_config& config);

} // namespace ws::core