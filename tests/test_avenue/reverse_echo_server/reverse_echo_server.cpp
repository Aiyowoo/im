//  reverse_echo_server.cpp
// Created by m8792 on 2019/4/21.
// 2019/4/21 1:15

#include "reverse_echo_server.hpp"
#include "logger.hpp"

void reverse_echo_server::on_receive_connection(boost::asio::ip::tcp::socket &socket) {
    auto conn_ptr = std::make_shared<connection_type>(std::move(socket), ssl_context_);
    conn_ptr->start([this, conn_ptr](const status &s) {
        if (!s) {
            ERROR_LOG("failed to initialize connection due to error[%s]", s.message().c_str());
            return;
        }
        DEBUG_LOG("initialize a connection successfully");
    });
}

void
reverse_echo_server::request_handler_type::operator()(std::shared_ptr<reverse_echo_server::connection_type> conn_ptr,
                                                      std::unique_ptr<avenue::message> msg,
                                                      boost::system::error_code ec) {
    if (ec) {
        ERROR_LOG("when waiting requests, encountered an error[%s]", ec.message().c_str());
        conn_ptr->close();
        return;
    }
    assert(conn_ptr && msg && msg->is_request());
    DEBUG_LOG("got {}", msg);

    char *body = nullptr;
    uint32_t body_len = 0;
    msg->get_body(body, body_len);
    std::reverse(body, body + body_len);
    msg->set_is_request(false);
    conn_ptr->send_response(std::move(msg));
}
