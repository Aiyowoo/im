//  server_connection.cpp
// Created by m8792 on 2019/4/19.
// 2019/4/19 22:03

#include "server_connection.hpp"

namespace avenue {

template<typename RequestHandler>
void server_connection<RequestHandler>::start() {
    auto self = shared_from_this();
    post([this, self] {
        do_start();
    });
}

template<typename RequestHandler>
void server_connection<RequestHandler>::on_receive_request(std::unique_ptr<message> request, const status &s) {
    auto self = shared_from_this();
    post([this, self, r = std::move(request), s]() mutable {
        do_receive_request(std::move(r), s);
    });
}

template<typename RequestHandler>
void server_connection<RequestHandler>::request(std::unique_ptr<message> request,
                                                message_connection_ops::request_callback_type handler) {
    message_ops_.send_request(std::move(request), handler);
}

template<typename RequestHandler>
void
server_connection<RequestHandler>::timed_request(std::unique_ptr<message> request,
                                                 message_connection_ops::request_handler_type handler,
                                                 uint32_t timeout_seconds) {
    message_ops_.timed_send_request(std::move(request), timeout_seconds, handler);
}

template<typename RequestHandler>
void server_connection<RequestHandler>::send_response(std::unique_ptr<message> response) {
    message_ops_.send_response(std::move(response));
}

template<typename RequestHandler>
void server_connection<RequestHandler>::close() {
    auto self = shared_from_this();
    post([this, self] {
        do_close();
    });
}

template<typename RequestHandler>
void server_connection<RequestHandler>::do_start() {
    auto self = shared_from_this();
    message_ops_.start([this, self](std::unique_ptr<message> request, const status &s) {
        on_receive_request(std::move(request), s);
    });
}

template<typename RequestHandler>
void server_connection<RequestHandler>::do_receive_request(std::unique_ptr<message> request,
                                                           const status &s) {
    request_handler_(*this, std::move(request), s);
}

template<typename RequestHandler>
void server_connection<RequestHandler>::do_close() {
    boost::system::error_code ec;
    stream_.next_layer().close(ec);
    message_ops_.stop();
}

}