//  server_connection.cpp
// Created by m8792 on 2019/4/24.
// 2019/4/24 22:17

#include "server_connection.hpp"
#include "details/log_helper.hpp"

namespace asio = boost::asio;
namespace ssl = boost::asio::ssl;

namespace avenue {

server_connection::server_connection(boost::asio::ip::tcp::socket &socket,
                                     boost::asio::ssl::context &ssl_context)
        : message_connection(socket, ssl_context) {
}

void server_connection::run() {
    INFO_LOG("start to run...");
    stream().async_handshake(ssl::stream_base::handshake_type::server,
                             [this, self = shared_from_base()](boost::system::error_code ec) {
                                 if (!ec) {
                                     initialize();
                                 }
                                 status s(ec.value(),
                                          fmt::format("failed to handshake with client due to error[{}]",
                                                      ec.message()));
                                 on_initialized(s);
                             });
}

std::shared_ptr<server_connection> server_connection::shared_from_base() {
    return std::dynamic_pointer_cast<server_connection>(shared_from_this());
}

}