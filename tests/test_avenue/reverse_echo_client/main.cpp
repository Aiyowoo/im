//  main.cpp
// Created by m8792 on 2019/4/21.
// 2019/4/21 15:50

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#endif

#include "reverse_echo_client_connection.hpp"
#include "logger.hpp"

#include <comm/status.hpp>
#include <avenue/message.hpp>
#include <avenue/client_connection.hpp>

#include <gtest/gtest.h>

#include <memory>
#include <string>

#include <boost/asio/ssl.hpp>

namespace ssl = boost::asio::ssl;

int main() {
    try {
        boost::asio::io_context context;
        boost::asio::ssl::context ssl_context(ssl::context::sslv23);
        ssl_context.set_verify_mode(ssl::context::verify_none);
//        auto conn_ptr = std::make_shared<reverse_echo_client_connection>(context, ssl_context);
//        conn_ptr->run("127.0.0.1", "54321");
//        context.run();
        using tcp = boost::asio::ip::tcp;
        ssl::stream<tcp::socket> stream(context, ssl_context);
        stream.lowest_layer().connect(tcp::endpoint(boost::asio::ip::address_v4::from_string("127.0.0.1"), 54321));
        stream.async_handshake(ssl::stream_base::client, [&stream](boost::system::error_code ec) {
            DEBUG_LOG("{}", ec.message());
        });
        context.run();

    } catch (const std::exception &e) {
        ERROR_LOG("encountered an exception[{}]", e.what());
    }
}