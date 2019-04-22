//  message_connection.cpp
// Created by m8792 on 2019/4/21.
// 2019/4/21 23:37

#include "message_connection.hpp"
#include "message.hpp"

#include "details/log_helper.hpp"

#ifdef DEBUG

#include <fmt/format.h>

#endif

namespace avenue {

bool message_connection::deadline_request_id_p_comp::operator()(const message_connection::deadline_request_id_p &lhs,
                                                                const message_connection::deadline_request_id_p &rhs) const {
    return lhs.deadline > rhs.deadline || (lhs.deadline == rhs.deadline && lhs.request_id > rhs.request_id);
}

message_connection::message_connection(boost::asio::ip::tcp::socket &socket,
                                       boost::asio::ssl::context &ssl_context)
        : stream_(std::move(socket), ssl_context), initialized_(false), read_closed_(true),
          write_closed_(true), want_close_write_(false), request_timer_(stream_.get_io_context()) {
#ifdef DEBUG
    d_create_time_ = get_current_time_str();
#endif
}

message_connection::message_connection(boost::asio::io_context &io_context,
                                       boost::asio::ssl::context &ssl_context)
        : stream_(io_context, ssl_context), initialized_(false), read_closed_(true),
          write_closed_(true), want_close_write_(false), request_timer_(stream_.get_io_context()) {
#ifdef DEBUG
    d_create_time_ = get_current_time_str();
#endif
}

message_connection::~message_connection() {
    close();
}

void message_connection::on_initialized(const status &s) {
    if (s) {
        initialized_ = true;
        read_closed_ = false;
        write_closed_ = false;
        want_close_write_ = false;
    }

#ifdef DEBUG
    d_initialized_time_ = get_current_time_str();
    if (s) {
        auto addr = stream_.next_layer().local_endpoint();
        d_local_addr_ = fmt::format("{}:{}", addr.address().to_string(), addr.port());
        addr = stream_.next_layer().remote_endpoint();
        d_remote_addr_ = fmt::format("{}:{}", addr.address().to_string(), addr.port());
    }
#endif
}


void message_connection::request(message *msg, request_callback_type handler) {
    assert(msg);
    post([this, self = shared_from_this(), msg, handler] {
        do_request(msg, handler);
    });
}

void message_connection::timed_request(message *msg, clock_type::duration timeout,
                                       request_callback_type handler) {
    assert(msg);
    clock_type::time_point deadline = clock_type::now() + timeout;
    post([this, self = shared_from_this(), msg, deadline, handler] {
        do_request(msg, deadline, handler);
    });
}

void message_connection::response(message *msg, message_connection::request_callback_type handler) {
    assert(msg);
    post([this, self = shared_from_this(), msg, handler] {
        do_response(msg, handler);
    });
}

void message_connection::close() {
    post([this, self = shared_from_this()] {
        do_close();
    });
}

void message_connection::post(std::function<void()> handler) {
    stream_.get_io_context().post(handler);
}

void message_connection::do_request(message *msg, request_callback_type handler) {
    assert(msg && msg->is_request());

    DEBUG_LOG("msg[{}]", msg);

    if (!initialized_ || want_close_write_ || write_closed_) {
        status s(status::HALF_CLOSED, "write closed or want close write");
        return;
    }

    // TODO: continue

}

#ifdef DEBUG

std::string message_connection::get_extra_log_info() {
    return fmt::format("create_time[{}] initialized_time[{}] local_addr[{}] remote_addr[{}] "
                       "sent_request_count[{}] received_response_count[{}] received_request_count[{}] "
                       "sent_response_count[{}] timeout_request_count[{}] initialized[{}] "
                       "read_closed[{}] write_closed[{}] want_close_write[{}]",
                       d_create_time_, d_initialized_time_, d_local_addr_, d_remote_addr_,
                       d_sent_request_count_, d_received_response_count_, d_received_request_count_,
                       d_sent_response_count_, d_timeout_request_count_, initialized_, read_closed_,
                       write_closed_, want_close_write_);
}

#endif

}
