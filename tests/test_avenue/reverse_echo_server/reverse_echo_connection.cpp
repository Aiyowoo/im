//  reverse_echo_connection.cpp
// Created by m8792 on 2019/4/24.
// 2019/4/24 23:45


#include "reverse_echo_connection.hpp"

#include "logger.hpp"

#include <avenue/message.hpp>

constexpr uint32_t IGNORE_SERVICE_ID = 302;
constexpr uint32_t IGNORE_MSG_ID = 302;

reverse_echo_connection::reverse_echo_connection(boost::asio::ip::tcp::socket &socket,
                                                 boost::asio::ssl::context &ssl_context)
        : server_connection(socket, ssl_context) {
}

void reverse_echo_connection::on_initialized(const status &s) {
    DEBUG_LOG("connection[{}] initialized, result code[{}] message[{}]",
              reinterpret_cast<void *>(this), s.code(), s.message());
}

void reverse_echo_connection::on_receive_request(avenue::message *msg) {
    DEBUG_LOG("connection[{}] received request[{}]", reinterpret_cast<void *>(this),
              reinterpret_cast<void *>(msg));
	// 对测试超时的消息直接丢弃
	if (msg->get_service_id() == IGNORE_SERVICE_ID) {
		DEBUG_LOG("received ignore message servie_id[{}] message_id[{}] sequence[{}]",
			msg->get_service_id(), msg->get_message_id(), msg->get_sequence());
		delete msg;
		return;
	}

    char *body = nullptr;
    size_t body_len = 0;
    msg->get_body(body, body_len);
    DEBUG_LOG("received: [{}]", std::string(body, body_len));

    std::reverse(body, body + body_len);
    DEBUG_LOG("response with: [{}]", std::string(body, body_len));

    msg->set_is_request(false);
    response(msg);
}

void reverse_echo_connection::on_closed() {
    DEBUG_LOG("connection[{}] closed", reinterpret_cast<void *>(this));
}

reverse_echo_connection::~reverse_echo_connection() {
    DEBUG_LOG("connection[{}] destructed...", reinterpret_cast<void *>(this));
}
