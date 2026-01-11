#pragma once

#include "ws_client/client_config.hpp"
#include "ws_common/interface.hpp"
#include <memory>
#include <string>
#include <functional>
#include <vector>
#include <map>
#include <mutex>

namespace KK_WS::client {

// 前向声明
class ClientImpl;

/**
 * @brief WebSocket客户端类
 *
 * 提供高级的WebSocket客户端功能，封装底层连接细节
 */
class WebSocketClient : public IWebSocketEndpoint {
public:
    /**
     * @brief 构造函数
     * @param client_id 客户端标识（可选）
     */
    explicit WebSocketClient(const std::string& client_id = "");

    /**
     * @brief 析构函数
     */
    ~WebSocketClient() override;

    // 禁止拷贝
    WebSocketClient(const WebSocketClient&) = delete;
    WebSocketClient& operator=(const WebSocketClient&) = delete;

    // 允许移动
    WebSocketClient(WebSocketClient&&) noexcept;
    WebSocketClient& operator=(WebSocketClient&&) noexcept;

    // ========== IWebSocketEndpoint接口实现 ==========

    /**
     * @brief 连接到WebSocket服务器
     * @param config 连接配置
     */
    bool connect(const ws_config& config) override;

    /**
     * @brief 使用客户端配置连接
     * @param config 客户端配置
     * @return 连接是否成功
     */
    bool connect(const ClientConfig& config);

    /**
     * @brief 断开连接
     */
    void disconnect() override;

    /**
     * @brief 获取当前连接状态
     */
    ws_connection_state get_connection_state() const override;

    /**
     * @brief 发送消息
     * @param message 要发送的消息
     */
    bool send_message(const ws_message& message) override;

    /**
     * @brief 发送文本消息（便捷方法）
     * @param text 文本内容
     */
    void send_text(const std::string& text);

    /**
     * @brief 发送二进制消息（便捷方法）
     * @param data 二进制数据
     */
    void send_binary(const std::string& data);

    // ========== 回调设置 ==========

    void set_message_callback(MessageCallback callback) override;
    void set_state_callback(StateCallback callback) override;
    void set_error_callback(ErrorCallback callback) override;

    // ========== 高级功能 ==========

    /**
     * @brief 订阅主题
     * @param topic 主题名称
     */
    void subscribe(const std::string& topic);

    /**
     * @brief 取消订阅
     * @param topic 主题名称
     */
    void unsubscribe(const std::string& topic);

    /**
     * @brief 检查是否已订阅主题
     */
    bool is_subscribed(const std::string& topic) const;

    /**
     * @brief 获取订阅的主题列表
     */
    std::vector<std::string> get_subscriptions() const;

    // ========== 状态信息 ==========

    /**
     * @brief 获取客户端ID
     */
    std::string get_client_id() const;

    /**
     * @brief 获取服务器地址
     */
    std::string get_server_uri() const;

    /**
     * @brief 获取连接持续时间（毫秒）
     */
    uint64_t get_connection_duration() const;

    /**
     * @brief 获取发送的消息数量
     */
    uint64_t get_messages_sent() const;

    /**
     * @brief 获取接收的消息数量
     */
    uint64_t get_messages_received() const;

    /**
     * @brief 检查是否已连接
     */
    bool is_connected() const {
        return get_connection_state() == ws_connection_state::WS_CONNECTED;
    }

private:
    std::unique_ptr<ClientImpl> impl_;
};

/**
 * @brief 客户端管理器（单例）
 *
 * 用于管理多个WebSocket客户端实例
 */
class ClientManager {
public:
    static ClientManager& instance();

    // 客户端管理
    std::shared_ptr<WebSocketClient> create_client(const std::string& client_id = "");
    bool remove_client(const std::string& client_id);
    std::shared_ptr<WebSocketClient> get_client(const std::string& client_id);

    // 批量操作
    std::vector<std::string> get_client_ids() const;
    size_t get_client_count() const;

    // 全局配置
    void set_global_config(const ClientConfig& config);
    const ClientConfig& get_global_config() const;

private:
    ClientManager() = default;

    std::map<std::string, std::shared_ptr<WebSocketClient>> clients_;
    ClientConfig global_config_;
    mutable std::mutex mutex_;
};

} // namespace KK_WS::client
