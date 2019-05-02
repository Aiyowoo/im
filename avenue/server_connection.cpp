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
	set_running(true);

    stream().async_handshake(ssl::stream_base::handshake_type::server,
                             [this, self = shared_from_base()](boost::system::error_code ec) {
                                 if (ec) {
                                     status s(ec.value(),
                                              fmt::format("failed to handshake with client due to error[{}]",
                                                          ec.message()));
                                     handle_initialize_error(s);
                                     return;
                                 }
                                 initialize();
                             });
}

std::shared_ptr<server_connection> server_connection::shared_from_base() {
    return std::dynamic_pointer_cast<server_connection>(shared_from_this());
}

void server_connection::handle_initialize_error(const status& error) {
    // 已经不再运行了
	set_running(false);

    // fixme: 是否需要关闭socket连接

	on_initialized(error);
}

}
