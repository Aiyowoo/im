//  client_connection.cpp
// Created by m8792 on 2019/4/20.
// 2019/4/20 1:16

#include "client_connection.hpp"

namespace avenue {

namespace asio = boost::asio;
namespace ip = asio::ip;
using tcp = ip::tcp;

namespace ssl = boost::asio::ssl;


client_connection::client_connection(boost::asio::io_context &context,
                                     boost::asio::ssl::context &ssl_context)
        : stream_(context, ssl_context), resolver_(context), message_ops_(stream_) {
}

void client_connection::async_connect(const std::string &host, const std::string &service,
                                      connect_handler_type handler) {
    resolver_.async_resolve(host, service, [this, self = shared_from_this(), handler](boost::system::error_code ec,
                                                                                      tcp::resolver::iterator it) {
        on_resolver(it, ec, handler);
    });
}

void client_connection::request(std::unique_ptr<message> msg,
                                message_connection_ops::request_callback_type handler) {
    message_ops_.send_request(std::move(msg), handler);
}

void client_connection::timed_request(std::unique_ptr<message> msg, uint32_t timeout_seconds,
                                      message_connection_ops::request_callback_type handler) {
    message_ops_.timed_send_request(std::move(msg), timeout_seconds, handler);
}

void client_connection::response(std::unique_ptr<message> msg) {
    message_ops_.send_response(std::move(msg));
}

void client_connection::close() {
    message_ops_.post([this, self = shared_from_this()] {
        stream_.next_layer().close();
        message_ops_.stop();
    });
}

void client_connection::on_resolver(boost::asio::ip::tcp::resolver::iterator it, boost::system::error_code ec,
                                    client_connection::connect_handler_type handler) {
    if (ec) {
        handler(ec);
        return;
    }

    asio::async_connect(stream_.next_layer(), it, tcp::resolver::iterator(),
                        [this, self = shared_from_this(), handler](boost::system::error_code ec,
                                                                   tcp::resolver::iterator) {
                            if (ec) {
                                handler(ec);
                                return;
                            }

                            stream_.async_handshake(ssl::stream_base::client,
                                                    [this, self, handler](boost::system::error_code ec) {
                                                        handler(ec);
                                                    });
                        });
}

}