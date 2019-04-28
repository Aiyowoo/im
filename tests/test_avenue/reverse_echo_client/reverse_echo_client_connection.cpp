//  reverse_echo_client_connection.cpp
// Created by m8792 on 2019/4/25.
// 2019/4/25 0:33

#include "reverse_echo_client_connection.hpp"

#include "logger.hpp"

#include <avenue/message.hpp>

reverse_echo_client_connection::reverse_echo_client_connection(boost::asio::io_context &context,
                                                               boost::asio::ssl::context &ssl_context)
        : avenue::client_connection(context, ssl_context) {
}


void reverse_echo_client_connection::on_initialized(const status &s) {
    DEBUG_LOG("connection[{}] initialized, result code[{}] message[{}]",
              reinterpret_cast<void *>(this), s.code(), s.message());
    if(!s) {
		return;
    }

    auto *msg = new avenue::message(1, 2);
    std::string data = "hello world";
    msg->prepare(data.size());
    char *body = nullptr;
    size_t body_len = 0;
    msg->get_body(body, body_len);
    std::copy(data.cbegin(), data.cend(), body);
    msg->set_is_request(true);
    request(msg, [this, self = shared_from_base()](avenue::message *resp, const status &s) {
        if (!s) {
            ERROR_LOG("failed to got response due to error[{}]", s.message());
            return;
        }

        char *body = nullptr;
        uint32_t body_len = 0;
        resp->get_body(body, body_len);
        DEBUG_LOG("got response: [{}]", std::string(body, body_len));

        delete resp;

        close();
    });
}

void reverse_echo_client_connection::on_receive_request(avenue::message *msg) {
    delete msg;
}

void reverse_echo_client_connection::on_closed() {
    DEBUG_LOG("connection[{}] closed", reinterpret_cast<void*>(this));
}

reverse_echo_client_connection::~reverse_echo_client_connection() {
    DEBUG_LOG("connection[{}] destructed", reinterpret_cast<void*>(this));
}