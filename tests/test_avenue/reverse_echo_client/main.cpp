//  main.cpp
// Created by m8792 on 2019/4/21.
// 2019/4/21 15:50

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#endif

#include "logger.hpp"

#include <comm/status.hpp>
#include <avenue/message.hpp>
#include <avenue/client_connection.hpp>

#include <gtest/gtest.h>

#include <memory>
#include <string>

namespace asio = boost::asio;
namespace ip = asio::ip;
using tcp = ip::tcp;
namespace ssl = boost::asio::ssl;

//TEST(reverse_echo_server, one_connection_one_message) {
//    try {
//        asio::io_context io_context;
//        ssl::context ssl_context(ssl::context::sslv23);
//        ssl_context.set_verify_mode(ssl::context::verify_none);
//        std::shared_ptr<avenue::client_connection> conn_ptr =
//                std::make_shared<avenue::client_connection>(io_context, ssl_context);
//        conn_ptr->async_connect("127.0.0.1", "54321", [conn_ptr](boost::system::error_code ec) {
//            if (ec) {
//                ERROR_LOG("failed to connect to server due to error[%s]", ec.message().c_str());
//                return;
//            }
//
//            std::unique_ptr<avenue::message> msg(new avenue::message(12, 1, 1));
//            msg->set_is_request(true);
//            std::string data = "hello world";
//            msg->prepare(data.size());
//            char *body = nullptr;
//            uint32_t body_len = 0;
//            msg->get_body(body, body_len);
//            ASSERT_EQ(data.size(), body_len);
//            std::copy(data.cbegin(), data.cend(), body);
//            std::string info = fmt::format("{}", *msg);
//            DEBUG_LOG("try send %s", info.c_str());
//            conn_ptr->request(std::move(msg), [conn_ptr](std::unique_ptr<avenue::message> msg, const status &s) {
//                if (!s) {
//                    ERROR_LOG("request error[%s]", s.message().c_str());
//                    conn_ptr->close();
//                    return;
//                }
//                DEBUG_LOG("got %s", fmt::format("{}", *msg).c_str());
//                conn_ptr->close();
//            });
//        });
//
//        io_context.run();
//    } catch(const std::exception &e) {
//        ERROR_LOG("encountered an exception[%s]", e.what());
//    }
//}

int main() {
    try {
        asio::io_context io_context;
        ssl::context ssl_context(ssl::context::sslv23);
        ssl_context.set_verify_mode(ssl::context::verify_none);
        std::shared_ptr<avenue::client_connection> conn_ptr =
                std::make_shared<avenue::client_connection>(io_context, ssl_context);
        conn_ptr->async_connect("127.0.0.1", "54321",
                                [conn_ptr](std::unique_ptr<avenue::message> m, const status &s) {
                                    if (!s) {
                                        ERROR_LOG("failed to connect to server due to error[%s]", s.message());
                                        return;
                                    }

                                    std::unique_ptr<avenue::message> msg(new avenue::message(12, 1, 1));
                                    msg->set_is_request(true);
                                    std::string data = "hello world";
                                    msg->prepare(data.size());
                                    char *body = nullptr;
                                    uint32_t body_len = 0;
                                    msg->get_body(body, body_len);
//            ASSERT_EQ(data.size(), body_len);
                                    std::copy(data.cbegin(), data.cend(), body);
                                    std::string info = fmt::format("{}", *msg);
                                    DEBUG_LOG("try send %s", info.c_str());
                                    conn_ptr->request(std::move(msg), [conn_ptr](std::unique_ptr<avenue::message> msg,
                                                                                 const status &s) {
                                        if (!s) {
                                            ERROR_LOG("request error[%s]", s.message().c_str());
                                            conn_ptr->close();
                                            return;
                                        }
                                        DEBUG_LOG("got %s", fmt::format("{}", *msg).c_str());
                                        conn_ptr->close();
                                    });
                                });

        io_context.run();
    } catch (const std::exception &e) {
        ERROR_LOG("encountered an exception[%s]", e.what());
    }
    return 0;
}