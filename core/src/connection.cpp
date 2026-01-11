#include "ws_core/connection.hpp"
#include "ws_common/interface.hpp"
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/client.hpp>
#include <websocketpp/server.hpp>

namespace ws::core {

class ConnectionImpl {
    // TODO: 实现连接逻辑
};

Connection::Connection() : impl(std::make_unique<ConnectionImpl>()) {}
Connection::~Connection() = default;

} // namespace ws::core