//  reverse_echo_server.hpp
// Created by m8792 on 2019/4/21.
// 2019/4/21 1:15

#ifndef TEST_REVERSE_ECHO_SERVER_HPP
#define TEST_REVERSE_ECHO_SERVER_HPP

#include <comm/status.hpp>
#include <avenue/tcp_server.hpp>
#include <avenue/server_connection.hpp>

#include <boost/system/error_code.hpp>
#include <boost/asio.hpp>

#include <memory>

class reverse_echo_server {
    avenue::tcp_server server_;
    boost::asio::ssl::context ssl_context_;

public:
    struct request_handler_type;

    using connection_type = avenue::server_connection<request_handler_type>;

public:

    struct request_handler_type {
        void operator()(std::shared_ptr<connection_type> conn_ptr, std::unique_ptr<avenue::message> msg,
                        boost::system::error_code ec);
    };

    void on_receive_connection(boost::asio::ip::tcp::socket &socket);
};

#endif //TEST_REVERSE_ECHO_SERVER_HPP
