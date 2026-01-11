#include "ws_server/server.hpp"
#include "ws_common/logger.hpp"
#include <algorithm>

namespace KK_WS::server {

WebSocketServer::WebSocketServer(const ServerConfig& config)
    : config_(config) {
    
    // 配置日志
    if (!config_.enable_logging) {
        endpoint_.clear_access_channels(websocketpp::log::alevel::all);
        endpoint_.clear_error_channels(websocketpp::log::elevel::all);
    } else {
        endpoint_.set_error_channels(websocketpp::log::elevel::all);
        endpoint_.set_access_channels(websocketpp::log::alevel::all ^ 
                                     websocketpp::log::alevel::frame_payload);
    }

    // 初始化ASIO
    endpoint_.init_asio();

    // 设置事件处理器
    endpoint_.set_open_handler([this](connection_hdl hdl) {
        on_open(hdl);
    });

    endpoint_.set_close_handler([this](connection_hdl hdl) {
        on_close(hdl);
    });

    endpoint_.set_message_handler([this](connection_hdl hdl, message_ptr msg) {
        on_message(hdl, msg);
    });

    // 设置复用地址
    endpoint_.set_reuse_addr(true);
}

WebSocketServer::~WebSocketServer() {
    stop();
}

void WebSocketServer::start() {
    try {
        Logger::info("启动WebSocket服务器...");
        Logger::info("监听端口: " + std::to_string(config_.port));
        Logger::info("绑定地址: " + config_.bind_address);

        // 监听指定端口
        endpoint_.listen(config_.port);

        // 开始接受连接
        endpoint_.start_accept();

        Logger::info("服务器启动成功，等待连接...");

        // 运行事件循环（阻塞）
        endpoint_.run();

    } catch (const websocketpp::exception& e) {
        Logger::error("WebSocket异常: " + std::string(e.what()));
    } catch (const std::exception& e) {
        Logger::error("启动服务器失败: " + std::string(e.what()));
    }
}

void WebSocketServer::stop() {
    try {
        Logger::info("停止WebSocket服务器...");
        
        // 关闭所有连接
        {
            std::lock_guard<std::mutex> lock(connections_mutex_);
            for (auto hdl : connections_) {
                websocketpp::lib::error_code ec;
                endpoint_.close(hdl, websocketpp::close::status::going_away, 
                              "服务器关闭", ec);
            }
            connections_.clear();
        }

        // 停止监听
        endpoint_.stop_listening();
        
        // 停止服务器
        endpoint_.stop();

        Logger::info("服务器已停止");

    } catch (const std::exception& e) {
        Logger::error("停止服务器失败: " + std::string(e.what()));
    }
}

void WebSocketServer::send_message(connection_hdl hdl, const std::string& message) {
    try {
        websocketpp::lib::error_code ec;
        endpoint_.send(hdl, message, websocketpp::frame::opcode::text, ec);
        
        if (ec) {
            Logger::error("发送消息失败: " + ec.message());
        }
    } catch (const std::exception& e) {
        Logger::error("发送消息异常: " + std::string(e.what()));
    }
}

void WebSocketServer::broadcast(const std::string& message) {
    std::lock_guard<std::mutex> lock(connections_mutex_);
    
    Logger::debug("广播消息到 " + std::to_string(connections_.size()) + " 个客户端");
    
    for (auto hdl : connections_) {
        send_message(hdl, message);
    }
}

void WebSocketServer::set_message_handler(MessageHandler handler) {
    message_handler_ = handler;
}

void WebSocketServer::set_open_handler(ConnectionHandler handler) {
    open_handler_ = handler;
}

void WebSocketServer::set_close_handler(ConnectionHandler handler) {
    close_handler_ = handler;
}

size_t WebSocketServer::get_connection_count() const {
    std::lock_guard<std::mutex> lock(connections_mutex_);
    return connections_.size();
}

void WebSocketServer::on_open(connection_hdl hdl) {
    {
        std::lock_guard<std::mutex> lock(connections_mutex_);
        connections_.insert(hdl);
    }

    auto con = endpoint_.get_con_from_hdl(hdl);
    Logger::info("新客户端连接: " + con->get_remote_endpoint() + 
                 " (总数: " + std::to_string(get_connection_count()) + ")");

    if (open_handler_) {
        open_handler_(hdl);
    }
}

void WebSocketServer::on_close(connection_hdl hdl) {
    {
        std::lock_guard<std::mutex> lock(connections_mutex_);
        connections_.erase(hdl);
    }

    Logger::info("客户端断开连接 (剩余: " + 
                 std::to_string(get_connection_count()) + ")");

    if (close_handler_) {
        close_handler_(hdl);
    }
}

void WebSocketServer::on_message(connection_hdl hdl, message_ptr msg) {
    const std::string& payload = msg->get_payload();
    
    Logger::debug("收到消息: " + payload.substr(0, std::min(size_t(50), payload.size())) + 
                  (payload.size() > 50 ? "..." : ""));

    if (message_handler_) {
        message_handler_(hdl, payload);
    } else {
        // 默认行为：回显消息
        send_message(hdl, payload);
    }
}

} // namespace KK_WS::server
