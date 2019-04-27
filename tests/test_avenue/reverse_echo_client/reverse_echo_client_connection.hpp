//  reverse_echo_client_connection.hpp
// Created by m8792 on 2019/4/25.
// 2019/4/25 0:33

#ifndef TEST_REVERSE_ECHO_CLIENT_CONNECTION_HPP
#define TEST_REVERSE_ECHO_CLIENT_CONNECTION_HPP

#include <avenue/client_connection.hpp>

class reverse_echo_client_connection : public avenue::client_connection {

public:
    reverse_echo_client_connection(boost::asio::io_context &context, boost::asio::ssl::context &ssl_context);

    void on_initialized(const status &s) override;

    void on_receive_request(avenue::message *msg) override;

    void on_closed() override;

    ~reverse_echo_client_connection() override;
};


#endif //TEST_REVERSE_ECHO_CLIENT_CONNECTION_HPP
