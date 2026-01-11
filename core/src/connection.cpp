#include "ws_core/connection.hpp"
#include "ws_common/logger.hpp"
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/client.hpp>
#include <websocketpp/server.hpp>
#include <thread>
#include <atomic>

namespace ws::core {

using client_t = websocketpp::client<websocketpp::config::asio>;
using connection_hdl = websocketpp::connection_hdl;

/**
 * @brief Connection的私有实现类（Pimpl模式）
 */
class Connection::ConnectionImpl {
public:
    ConnectionImpl() 
        : state_(ws_connection_state::WS_DISCONNECTED)
        , reconnect_attempts_(0) {
        
        // 初始化WebSocket客户端
        client_.clear_access_channels(websocketpp::log::alevel::all);
        client_.clear_error_channels(websocketpp::log::elevel::all);
        
        client_.init_asio();
        
        // 设置事件处理器
        client_.set_open_handler([this](connection_hdl hdl) {
            on_open(hdl);
        });
        
        client_.set_close_handler([this](connection_hdl hdl) {
            on_close(hdl);
        });
        
        client_.set_fail_handler([this](connection_hdl hdl) {
            on_fail(hdl);
        });
        
        client_.set_message_handler([this](connection_hdl hdl, client_t::message_ptr msg) {
            on_message(hdl, msg);
        });
    }

    ~ConnectionImpl() {
        disconnect();
    }

    void connect() {
        if (state_ == ws_connection_state::WS_CONNECTED || 
            state_ == ws_connection_state::WS_CONNECTING) {
            Logger::warning("已经连接或正在连接中");
            return;
        }

        if (!config_.validate()) {
            Logger::error("WebSocket配置无效");
            notify_error("配置无效");
            return;
        }

        try {
            state_ = ws_connection_state::WS_CONNECTING;
            notify_state_change(state_);
            
            websocketpp::lib::error_code ec;
            client_t::connection_ptr con = client_.get_connection(config_.uri, ec);
            
            if (ec) {
                Logger::error("创建连接失败: " + ec.message());
                state_ = ws_connection_state::WS_FAILED;
                notify_error(ec.message());
                return;
            }

            hdl_ = con->get_handle();
            client_.connect(con);
            
            // 在新线程中运行事件循环
            if (!io_thread_.joinable()) {
                io_thread_ = std::thread([this]() {
                    try {
                        client_.run();
                    } catch (const std::exception& e) {
                        Logger::error("IO线程异常: " + std::string(e.what()));
                    }
                });
            }
            
            Logger::info("正在连接到: " + config_.uri);
            
        } catch (const std::exception& e) {
            Logger::error("连接异常: " + std::string(e.what()));
            state_ = ws_connection_state::WS_FAILED;
            notify_error(e.what());
        }
    }

    void disconnect() {
        if (state_ == ws_connection_state::WS_DISCONNECTED) {
            return;
        }

        state_ = ws_connection_state::WS_DISCONNICTING;
        notify_state_change(state_);

        try {
            websocketpp::lib::error_code ec;
            client_.close(hdl_, websocketpp::close::status::normal, "断开连接", ec);
            
            if (ec) {
                Logger::error("关闭连接失败: " + ec.message());
            }
            
            client_.stop();
            
            if (io_thread_.joinable()) {
                io_thread_.join();
            }
            
            state_ = ws_connection_state::WS_DISCONNECTED;
            notify_state_change(state_);
            
        } catch (const std::exception& e) {
            Logger::error("断开连接异常: " + std::string(e.what()));
        }
    }

    ws_connection_state get_state() const {
        return state_;
    }

    void send_message(const ws_message& message) {
        if (state_ != ws_connection_state::WS_CONNECTED) {
            Logger::warning("未连接，无法发送消息");
            return;
        }

        try {
            websocketpp::lib::error_code ec;
            
            auto opcode = (message.type == ws_message::message_type::TEXT) 
                ? websocketpp::frame::opcode::text 
                : websocketpp::frame::opcode::binary;
            
            client_.send(hdl_, message.payload, opcode, ec);
            
            if (ec) {
                Logger::error("发送消息失败: " + ec.message());
                notify_error(ec.message());
            }
            
        } catch (const std::exception& e) {
            Logger::error("发送消息异常: " + std::string(e.what()));
        }
    }

    void set_config(const ws_config& config) {
        config_ = config;
    }

    const ws_config& get_config() const {
        return config_;
    }

    void set_message_callback(MessageCallback cb) {
        message_callback_ = cb;
    }

    void set_state_callback(StateCallback cb) {
        state_callback_ = cb;
    }

    void set_error_callback(ErrorCallback cb) {
        error_callback_ = cb;
    }

private:
    void on_open(connection_hdl hdl) {
        Logger::info("WebSocket连接已建立");
        state_ = ws_connection_state::WS_CONNECTED;
        reconnect_attempts_ = 0;
        notify_state_change(state_);
    }

    void on_close(connection_hdl hdl) {
        Logger::info("WebSocket连接已关闭");
        state_ = ws_connection_state::WS_DISCONNECTED;
        notify_state_change(state_);
    }

    void on_fail(connection_hdl hdl) {
        Logger::error("WebSocket连接失败");
        state_ = ws_connection_state::WS_FAILED;
        notify_state_change(state_);
        notify_error("连接失败");
    }

    void on_message(connection_hdl hdl, client_t::message_ptr msg) {
        if (message_callback_) {
            auto msg_type = (msg->get_opcode() == websocketpp::frame::opcode::text)
                ? ws_message::message_type::TEXT
                : ws_message::message_type::BINARY;
            
            ws_message ws_msg(msg_type, msg->get_payload(), 0);
            message_callback_(ws_msg);
        }
    }

    void notify_state_change(ws_connection_state state) {
        if (state_callback_) {
            state_callback_(state);
        }
    }

    void notify_error(const std::string& error) {
        if (error_callback_) {
            error_callback_(error);
        }
    }

private:
    client_t client_;
    connection_hdl hdl_;
    std::thread io_thread_;
    
    ws_config config_;
    std::atomic<ws_connection_state> state_;
    int reconnect_attempts_;

    MessageCallback message_callback_;
    StateCallback state_callback_;
    ErrorCallback error_callback_;
};

// Connection类的实现
Connection::Connection() 
    : impl_(std::make_unique<ConnectionImpl>()) {
}

Connection::~Connection() = default;

void Connection::connect() {
    impl_->connect();
}

void Connection::disconnect() {
    impl_->disconnect();
}

ws_connection_state Connection::get_connection_state() const {
    return impl_->get_state();
}

void Connection::send_message(const ws_message& message) {
    impl_->send_message(message);
}

void Connection::set_message_callback(MessageCallback callback) {
    impl_->set_message_callback(callback);
}

void Connection::set_state_callback(StateCallback callback) {
    impl_->set_state_callback(callback);
}

void Connection::set_error_callback(ErrorCallback callback) {
    impl_->set_error_callback(callback);
}

void Connection::set_config(const ws_config& config) {
    impl_->set_config(config);
}

const ws_config& Connection::get_config() const {
    return impl_->get_config();
}

// 工厂函数
std::unique_ptr<IWebSocketEndpoint> create_connection(const ws_config& config) {
    auto conn = std::make_unique<Connection>();
    conn->set_config(config);
    return conn;
}

} // namespace ws::core