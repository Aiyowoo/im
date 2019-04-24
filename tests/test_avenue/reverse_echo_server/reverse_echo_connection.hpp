//  reverse_echo_connection.hpp
// Created by m8792 on 2019/4/24.
// 2019/4/24 23:45

#ifndef TEST_REVERSE_ECHO_CONNECTION_HPP
#define TEST_REVERSE_ECHO_CONNECTION_HPP

#include <avenue/message_connection.hpp>
#include <avenue/server_connection.hpp>

class reverse_echo_connection : public avenue::server_connection {

public:
    reverse_echo_connection(boost::asio::ip::tcp::socket &socket, boost::asio::ssl::context &ssl_context);

    void on_initialized(const status &s) override;

    void on_receive_request(avenue::message *msg) override;

    void on_closed() override;

    ~reverse_echo_connection();
};


#endif //TEST_REVERSE_ECHO_CONNECTION_HPP
