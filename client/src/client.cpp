#include "ws_client/client.hpp"
#include "ws_core/connection.hpp"
#include "ws_common/logger.hpp"
#include <thread>
#include <chrono>
#include <atomic>
#include <set>

namespace KK_WS::client {

// ========== ClientImpl 私有实现类 ==========

class ClientImpl {
public:
    ClientImpl(const std::string& client_id)
        : client_id_(client_id.empty() ? generate_client_id() : client_id)
        , reconnect_attempts_(0)
        , messages_sent_(0)
        , messages_received_(0)
        , connection_start_time_(0) {

        Logger::debug("创建客户端: " + client_id_);
    }

    ~ClientImpl() {
        disconnect_internal();
    }

    // 连接管理
    bool connect(const ClientConfig& config) {
        std::lock_guard<std::mutex> lock(mutex_);

        if (connection_) {
            Logger::warning("客户端 " + client_id_ + " 已连接，先断开");
            disconnect_internal();
        }

        // 保存配置
        config_ = config;

        // 创建底层连接
        ws_config ws_cfg;
        ws_cfg.uri = config.get_full_uri();
        ws_cfg.enable_auto_reconnect = config.auto_reconnect;
        ws_cfg.ping_interval_ms = config.ping_interval_ms;
        ws_cfg.reconnect_interval_ms = config.reconnect_interval_ms;

        connection_ = core::create_connection(ws_cfg);

        // 设置回调
        connection_->set_message_callback([this](const ws_message& msg) {
            on_message_received(msg);
        });

        connection_->set_state_callback([this](ws_connection_state state) {
            on_state_changed(state);
        });

        connection_->set_error_callback([this](const std::string& error) {
            on_error_occurred(error);
        });

        // 发起连接
        Logger::info("客户端 " + client_id_ + " 正在连接: " + ws_cfg.uri);
        connection_->connect(ws_cfg);

        // 等待连接完成（带超时）
        auto start = std::chrono::steady_clock::now();
        while (connection_->get_connection_state() == ws_connection_state::WS_CONNECTING) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));

            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - start);

            if (elapsed.count() > config.connect_timeout_ms) {
                Logger::error("连接超时: " + client_id_);
                return false;
            }
        }

        return connection_->get_connection_state() == ws_connection_state::WS_CONNECTED;
    }

    bool connect(const ws_config& config) {
        ClientConfig client_cfg;
        client_cfg.server_uri = config.uri;
        client_cfg.server_port = config.port;
        client_cfg.auto_reconnect = config.enable_auto_reconnect;
        client_cfg.ping_interval_ms = config.ping_interval_ms;
        client_cfg.reconnect_interval_ms = config.reconnect_interval_ms;

        return connect(client_cfg);
    }

    void disconnect() {
        std::lock_guard<std::mutex> lock(mutex_);
        disconnect_internal();
    }

    ws_connection_state get_connection_state() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return connection_ ? connection_->get_connection_state()
                           : ws_connection_state::WS_DISCONNECTED;
    }

    // 消息发送
    bool send_message(const ws_message& message) {
        std::lock_guard<std::mutex> lock(mutex_);

        if (!connection_ || connection_->get_connection_state() != ws_connection_state::WS_CONNECTED) {
            Logger::warning("客户端未连接，无法发送消息");
            return false;
        }

        connection_->send_message(message);
        messages_sent_++;
        return true;
    }

    void send_text(const std::string& text) {
        ws_message msg(ws_message::message_type::TEXT, text, 0);
        send_message(msg);
    }

    void send_binary(const std::string& data) {
        ws_message msg(ws_message::message_type::BINARY, data, 0);
        send_message(msg);
    }

    // 回调设置
    void set_message_callback(MessageCallback callback) {
        std::lock_guard<std::mutex> lock(mutex_);
        message_callback_ = std::move(callback);
    }

    void set_state_callback(StateCallback callback) {
        std::lock_guard<std::mutex> lock(mutex_);
        state_callback_ = std::move(callback);
    }

    void set_error_callback(ErrorCallback callback) {
        std::lock_guard<std::mutex> lock(mutex_);
        error_callback_ = std::move(callback);
    }

    // 订阅管理
    void subscribe(const std::string& topic) {
        std::lock_guard<std::mutex> lock(mutex_);
        subscriptions_.insert(topic);

        // 发送订阅消息到服务器
        if (connection_ && connection_->get_connection_state() == ws_connection_state::WS_CONNECTED) {
            send_text("SUBSCRIBE:" + topic);
        }
    }

    void unsubscribe(const std::string& topic) {
        std::lock_guard<std::mutex> lock(mutex_);
        subscriptions_.erase(topic);

        // 发送取消订阅消息
        if (connection_ && connection_->get_connection_state() == ws_connection_state::WS_CONNECTED) {
            send_text("UNSUBSCRIBE:" + topic);
        }
    }

    bool is_subscribed(const std::string& topic) const {
        std::lock_guard<std::mutex> lock(mutex_);
        return subscriptions_.find(topic) != subscriptions_.end();
    }

    std::vector<std::string> get_subscriptions() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return {subscriptions_.begin(), subscriptions_.end()};
    }

    // 状态信息
    std::string get_client_id() const {
        return client_id_;
    }

    std::string get_server_uri() const {
        return config_.get_full_uri();
    }

    uint64_t get_connection_duration() const {
        if (connection_start_time_ == 0) return 0;

        auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
                       std::chrono::system_clock::now().time_since_epoch()).count();

        return now - connection_start_time_;
    }

    uint64_t get_messages_sent() const {
        return messages_sent_;
    }

    uint64_t get_messages_received() const {
        return messages_received_;
    }

private:
    void disconnect_internal() {
        if (connection_) {
            connection_->disconnect();
            connection_.reset();
        }
        connection_start_time_ = 0;
    }

    void on_message_received(const ws_message& msg) {
        messages_received_++;

        // 调用用户回调
        if (message_callback_) {
            message_callback_(msg);
        }

        // 记录日志
        if (config_.verbose_logging) {
            std::string log_msg = "客户端 " + client_id_ + " 收到消息: ";
            if (msg.type == ws_message::message_type::TEXT) {
                log_msg += msg.payload.substr(0, 50);
                if (msg.payload.size() > 50) log_msg += "...";
            } else {
                log_msg += "[二进制数据 " + std::to_string(msg.payload.size()) + " 字节]";
            }
            Logger::debug(log_msg);
        }
    }

    void on_state_changed(ws_connection_state state) {
        // 更新连接开始时间
        if (state == ws_connection_state::WS_CONNECTED) {
            connection_start_time_ = std::chrono::duration_cast<std::chrono::milliseconds>(
                                         std::chrono::system_clock::now().time_since_epoch()).count();
        } else if (state == ws_connection_state::WS_DISCONNECTED) {
            connection_start_time_ = 0;
        }

        // 调用用户回调
        if (state_callback_) {
            state_callback_(state);
        }

        Logger::info("客户端 " + client_id_ + " 状态变更: " + state_to_string(state));
    }

    void on_error_occurred(const std::string& error) {
        if (error_callback_) {
            error_callback_(error);
        }

        Logger::error("客户端 " + client_id_ + " 错误: " + error);
    }

    static std::string generate_client_id() {
        static std::atomic<int> counter{0};
        return "client_" + std::to_string(++counter);
    }

    static std::string state_to_string(ws_connection_state state) {
        switch (state) {
        case ws_connection_state::WS_DISCONNECTED: return "断开连接";
        case ws_connection_state::WS_CONNECTING: return "连接中";
        case ws_connection_state::WS_CONNECTED: return "已连接";
        case ws_connection_state::WS_DISCONNICTING: return "断开中";
        case ws_connection_state::WS_FAILED: return "连接失败";
        default: return "未知";
        }
    }

private:
    mutable std::mutex mutex_;
    std::string client_id_;
    ClientConfig config_;
    std::unique_ptr<IWebSocketEndpoint> connection_;
    std::set<std::string> subscriptions_;

    // 回调
    MessageCallback message_callback_;
    StateCallback state_callback_;
    ErrorCallback error_callback_;

    // 统计信息
    std::atomic<int> reconnect_attempts_;
    std::atomic<uint64_t> messages_sent_;
    std::atomic<uint64_t> messages_received_;
    std::atomic<uint64_t> connection_start_time_;
};

// ========== WebSocketClient 公共接口实现 ==========

WebSocketClient::WebSocketClient(const std::string& client_id)
    : impl_(std::make_unique<ClientImpl>(client_id)) {
}

WebSocketClient::~WebSocketClient() = default;

WebSocketClient::WebSocketClient(WebSocketClient&& other) noexcept = default;
WebSocketClient& WebSocketClient::operator=(WebSocketClient&& other) noexcept = default;

bool WebSocketClient::connect(const ws_config& config) {
    return impl_->connect(config);
}

bool WebSocketClient::connect(const ClientConfig& config) {
    return impl_->connect(config);
}

void WebSocketClient::disconnect() {
    impl_->disconnect();
}

ws_connection_state WebSocketClient::get_connection_state() const {
    return impl_->get_connection_state();
}

bool WebSocketClient::send_message(const ws_message& message) {
    return impl_->send_message(message);
}

void WebSocketClient::send_text(const std::string& text) {
    impl_->send_text(text);
}

void WebSocketClient::send_binary(const std::string& data) {
    impl_->send_binary(data);
}

void WebSocketClient::set_message_callback(MessageCallback callback) {
    impl_->set_message_callback(std::move(callback));
}

void WebSocketClient::set_state_callback(StateCallback callback) {
    impl_->set_state_callback(std::move(callback));
}

void WebSocketClient::set_error_callback(ErrorCallback callback) {
    impl_->set_error_callback(std::move(callback));
}

void WebSocketClient::subscribe(const std::string& topic) {
    impl_->subscribe(topic);
}

void WebSocketClient::unsubscribe(const std::string& topic) {
    impl_->unsubscribe(topic);
}

bool WebSocketClient::is_subscribed(const std::string& topic) const {
    return impl_->is_subscribed(topic);
}

std::vector<std::string> WebSocketClient::get_subscriptions() const {
    return impl_->get_subscriptions();
}

std::string WebSocketClient::get_client_id() const {
    return impl_->get_client_id();
}

std::string WebSocketClient::get_server_uri() const {
    return impl_->get_server_uri();
}

uint64_t WebSocketClient::get_connection_duration() const {
    return impl_->get_connection_duration();
}

uint64_t WebSocketClient::get_messages_sent() const {
    return impl_->get_messages_sent();
}

uint64_t WebSocketClient::get_messages_received() const {
    return impl_->get_messages_received();
}

// ========== ClientManager 实现 ==========

ClientManager& ClientManager::instance() {
    static ClientManager instance;
    return instance;
}

std::shared_ptr<WebSocketClient> ClientManager::create_client(const std::string& client_id) {
    std::lock_guard<std::mutex> lock(mutex_);

    std::string id = client_id.empty() ? "client_" + std::to_string(clients_.size() + 1) : client_id;

    if (clients_.find(id) != clients_.end()) {
        Logger::warning("客户端已存在: " + id);
        return nullptr;
    }

    auto client = std::make_shared<WebSocketClient>(id);
    clients_[id] = client;

    Logger::info("创建客户端: " + id);
    return client;
}

bool ClientManager::remove_client(const std::string& client_id) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = clients_.find(client_id);
    if (it == clients_.end()) {
        Logger::warning("客户端不存在: " + client_id);
        return false;
    }

    // 断开连接
    it->second->disconnect();
    clients_.erase(it);

    Logger::info("移除客户端: " + client_id);
    return true;
}

std::shared_ptr<WebSocketClient> ClientManager::get_client(const std::string& client_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = clients_.find(client_id);
    return it != clients_.end() ? it->second : nullptr;
}

std::vector<std::string> ClientManager::get_client_ids() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::string> ids;
    ids.reserve(clients_.size());

    for (const auto& pair : clients_) {
        ids.push_back(pair.first);
    }

    return ids;
}

size_t ClientManager::get_client_count() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return clients_.size();
}

void ClientManager::set_global_config(const ClientConfig& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    global_config_ = config;
}

const ClientConfig& ClientManager::get_global_config() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return global_config_;
}

} // namespace KK_WS::client
