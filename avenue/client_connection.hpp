//  client_connection.hpp
// Created by m8792 on 2019/4/20.
// 2019/4/20 1:16

#ifndef AVENUE_CLIENT_CONNECTION_HPP
#define AVENUE_CLIENT_CONNECTION_HPP

#include "message_connection_ops.hpp"

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

#include <memory>

namespace avenue {

class client_connection : public std::enable_shared_from_this<client_connection> {
    using stream_type = boost::asio::ssl::stream<boost::asio::ip::tcp::socket>;

    using connect_handler_type = std::function<void(boost::system::error_code ec)>;

    stream_type stream_;

    boost::asio::ip::tcp::resolver resolver_;

    message_connection_ops message_ops_;

public:
    client_connection(boost::asio::io_context &context,
                      boost::asio::ssl::context &ssl_context);

    // 连接并完成ssl握手
    void async_connect(const std::string &host, const std::string &service,
                       connect_handler_type handler);

    void request(std::unique_ptr<message> msg, message_connection_ops::request_callback_type);

    void timed_request(std::unique_ptr<message> msg, uint32_t timeout_seconds,
                       message_connection_ops::request_callback_type handler);

    void response(std::unique_ptr<message> msg);

    void close();

private:
    void on_resolver(boost::asio::ip::tcp::resolver::iterator it,
                     boost::system::error_code ec, connect_handler_type handler);
};

}

#endif //AVENUE_CLIENT_CONNECTION_HPP
