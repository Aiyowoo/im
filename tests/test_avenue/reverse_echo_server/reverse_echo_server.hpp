//  reverse_echo_server.hpp
// Created by m8792 on 2019/4/21.
// 2019/4/21 1:15

#ifndef TEST_REVERSE_ECHO_SERVER_HPP
#define TEST_REVERSE_ECHO_SERVER_HPP

#include <comm/status.hpp>
#include <avenue/tcp_server.hpp>
#include <avenue/message.hpp>
#include <avenue/server_connection.hpp>

#include <boost/system/error_code.hpp>
#include <boost/asio.hpp>

#include <memory>

class reverse_echo_server {
    avenue::tcp_server server_;
    boost::asio::ssl::context ssl_context_;

public:
    reverse_echo_server();

    void run();

    void on_receive_connection(boost::asio::ip::tcp::socket &socket);
};

#endif //TEST_REVERSE_ECHO_SERVER_HPP
