#pragma once
#include <string>
#include <functional>

namespace KK_WS { // 创建websockets的命名空间
    // 枚举ws的连接状态
    enum class ws_connection_state{
        WS_DISCONNECTED, // 断开连接
        WS_CONNECTING,   // 连接中
        WS_CONNECTED,     // 已连接
        WS_DISCONNICTING,  // 断开连接中
        WS_FAILED        // 连接失败
    };

    // 配置结构
    struct ws_config{
        std::string uri; // WebSocket服务器的URI
        std::string host = "localhost"; // 主机名
        uint16_t port = 9002; // 端口号，默认9002
        bool use_ssl = false; // 是否使用SSL，默认不使用
        bool enable_auto_reconnect = true; // 是否启用自动重连，默认启用
        int ping_interval_ms = 10000; // 心跳间隔时间，单位毫秒，默认10秒
        int reconnect_interval_ms = 5000; // 重连间隔时间，单位毫秒，默认5秒
        int max_reconnect_attempts = 5; // 最大重连次数，默认5次

        // 验证配置有效性
        bool validate() const {
            return !uri.empty() || (!host.empty() && port > 0);
        }
    };

    // websocket消息结构
    struct ws_message{
        enum class message_type{ // 消息类型枚举
            TEXT,   // 文本消息
            BINARY, // 二进制消息
            PING,   //  心跳消息
            PONG,   // 心跳响应消息
            CLOSE   //  关闭连接消息
        };

        message_type type; // 消息类型
        std::string payload; // 消息内容
        uint16_t time_stamp; // 时间戳

        // 构造函数
        ws_message(message_type t, const std::string& p, uint16_t ts)
            : type(t), payload(p), time_stamp(ts) {}
        
    };

// 回调类型定义
using MessageCallback = std::function<void(const ws_message&)>;
using StateCallback = std::function<void(ws_connection_state)>;
using ErrorCallback = std::function<void(const std::string&)>;

    // IWebSocketEndpoint接口类
    class IWebSocketEndpoint{
    public:
        virtual ~IWebSocketEndpoint() = default; // 默认析构函数

        // 连接管理
        virtual bool connect(const ws_config& config) = 0; // 连接到WebSocket服务器
        virtual void disconnect() = 0; // 断开与WebSocket服务器的连接
        virtual ws_connection_state get_connection_state() const = 0; // 获取当前连接状态

        // 消息处理
        virtual bool send_message(const ws_message& message) = 0; // 发送消息

        // 回调设置
        virtual void set_message_callback(MessageCallback callback) = 0; // 设置消息回调
        virtual void set_state_callback(StateCallback callback) = 0; // 设置状态回调
        virtual void set_error_callback(ErrorCallback callback) = 0; // 设置错误回调
        
    };
}
