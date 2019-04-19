//  message_connection.cpp
// Created by m8792 on 2019/4/17.
// 2019/4/17 21:49

#include "message_connection_ops.hpp"
#include "message.hpp"

namespace avenue {

bool message_connection_ops::timer_pair_comp::operator()(const message_connection_ops::timer_pair &lhs,
                                                         const message_connection_ops::timer_pair &rhs) {
    return lhs.deadline > rhs.deadline;
}

message_connection_ops::message_connection_ops(message_connection_ops::stream_type &stream)
        : stream_(stream), timer_(stream_.get_io_context()), request_deadlines_(),
          tick_count_(0), received_message_(nullptr), want_stop_(false),
          is_writing_(false), is_reading_(false), is_timer_running_(false) {
}

message_connection_ops::~message_connection_ops() {
    do_stop();
}

void message_connection_ops::send_request(std::unique_ptr<message> request) {
    send_request(std::move(request), nullptr);
}

void message_connection_ops::send_request(std::unique_ptr<message> request,
                                          message_connection_ops::request_callback_type handler) {
    message *msg = request.release();
    post([this, msg, handler]() mutable {
        do_send_request(std::unique_ptr<message>(msg), std::move(handler));
    });
}

void message_connection_ops::timed_send_request(std::unique_ptr<message> request, uint32_t timeout_seconds,
                                                message_connection_ops::request_callback_type handler) {
    message *msg = request.release();
    post([this, msg, timeout_seconds, handler]() mutable {
        do_timed_send_request(std::unique_ptr<message>(msg), timeout_seconds, handler);
    });
}

void message_connection_ops::send_response(std::unique_ptr<message> response) {
    message *msg = response.release();
    post([this, msg]() mutable {
        do_send_response(std::unique_ptr<message>(msg));
    });
}

void message_connection_ops::stop() {
    post([this] {
        do_stop();
    });
}

void message_connection_ops::do_send_request(std::unique_ptr<message> request,
                                             message_connection_ops::request_callback_type handler) {
    uint32_t seq = request->get_sequence();
    assert(wait_response_callbacks_.find(seq) == wait_response_callbacks_.end());

    message *msg = request.release();
    wait_send_messages_.push_back(msg);
    wait_response_callbacks_[seq] = std::move(handler);

    start_send_if_need();
}

void message_connection_ops::do_timed_send_request(std::unique_ptr<message> request, uint32_t timeout_seconds,
                                                   message_connection_ops::request_callback_type handler) {
    auto deadline = std::chrono::system_clock::now() + std::chrono::seconds(timeout_seconds);
    uint32_t seq = request->get_sequence();
    request_deadlines_.push(timer_pair{deadline, seq});
    if (request_deadlines_.size() == 1) {
        is_timer_running_ = true;
        do_timer();
    }

    do_send_request(std::move(request), std::move(handler));
}

void message_connection_ops::do_send_response(std::unique_ptr<message> response) {
    wait_send_messages_.push_back(response.release());

    start_send_if_need();
}

void message_connection_ops::do_stop() {
    // cancel async_read, cancel async_write
    want_stop_ = true;

    boost::system::error_code ignore;
    timer_.cancel(ignore);

    while (is_reading_ || is_writing_ || is_timer_running_) {
        stream_.get_io_context().run_one();
    }

    for (message *msg:wait_send_messages_) {
        delete msg;
    }
    wait_send_messages_.clear();

    std::priority_queue<timer_pair, std::deque<timer_pair>, timer_pair_comp> tmp;
    request_deadlines_.swap(tmp);

    status s(status::OPERATION_CANCELLED, "operation cancelled");
    for (auto &p:wait_response_callbacks_) {
        p.second(nullptr, s);
    }
    wait_response_callbacks_.clear();

    if (received_message_) {
        delete received_message_;
        received_message_ = nullptr;
    }

    request_handler_ = nullptr;
}

void message_connection_ops::start_send_if_need() {
    if (wait_send_messages_.size() == 1) {
        send_message();
    }
}

void message_connection_ops::send_message() {
    message *msg = nullptr;
    while (!wait_send_messages_.empty() && msg == nullptr) {
        // 跳过已经超时，但是还没有发出去的请求
        msg = wait_send_messages_.front();
        if (msg->is_request() &&
            wait_response_callbacks_.find(msg->get_sequence()) == wait_response_callbacks_.cend()) {
            delete msg;
            msg = nullptr;
            wait_send_messages_.pop_front();
        }
    }

    if (msg == nullptr) {
        return;
    }

    is_writing_ = true;

    // 在尝试写message header之前，检查是否需要停止
    if (want_stop_) {
        is_writing_ = false;
        return;
    }

    boost::asio::const_buffer buffer(reinterpret_cast<const char *>(msg->get_header()),
                                     sizeof(message_header));
    boost::asio::async_write(stream_, buffer, boost::asio::transfer_all(),
                             [this](boost::system::error_code ec, size_t bytes_send) {
                                 on_send_message_header(ec, bytes_send);
                             });
}

void message_connection_ops::on_send_message_header(boost::system::error_code ec,
                                                    size_t bytes_send) {
    if (ec) {
        handle_message_sent(ec);
        return;
    }

    // 在尝试进行写body之前判断是否需要停止
    if (want_stop_) {
        is_writing_ = false;
        return;
    }

    assert(!wait_send_messages_.empty());

    const char *data = nullptr;
    size_t data_len = 0;
    wait_send_messages_.front()->get_body(data, data_len);
    boost::asio::async_write(stream_, boost::asio::const_buffer(data, data_len),
                             boost::asio::transfer_all(),
                             [this](boost::system::error_code ec, size_t bytes_send) {
                                 on_send_message_body(ec, bytes_send);
                             });
}

void message_connection_ops::on_send_message_body(boost::system::error_code ec, size_t bytes_send) {
    handle_message_sent(ec);
}

void message_connection_ops::handle_message_sent(boost::system::error_code ec) {
    assert(!wait_send_messages_.empty());

    uint32_t seq = wait_send_messages_.front()->get_sequence();

    delete wait_send_messages_.front();
    wait_send_messages_.pop_front();

    if (ec) {
        // 发送请求出错，调用请求的回调
        status s(ec.value(), "failed to send request due to error[" + ec.message() + "]");
        on_receive_response(seq, nullptr, s);
        is_writing_ = false;
        return;
    }

    // 如果上次发送成功，则继续发送
    send_message();
}

void message_connection_ops::on_receive_response(uint32_t request_id,
                                                 std::unique_ptr<message> response,
                                                 const status &s) {
    assert(!response || request_id == response->get_sequence());

    auto it = wait_response_callbacks_.find(request_id);
    if (it == wait_response_callbacks_.cend()) {
        return;
    }
    auto handler = it->second;
    wait_response_callbacks_.erase(it);

    handler(std::move(response), s);
}

void message_connection_ops::start(message_connection_ops::request_handler_type request_handler) {
    post([this, request_handler] {
        do_start(request_handler);
    });
}

void message_connection_ops::do_start(request_handler_type request_handler) {
    request_handler_ = std::move(request_handler);
    want_stop_ = false;
    is_reading_ = true;
    is_writing_ = is_timer_running_ = false;
    read_message();
}

void message_connection_ops::read_message() {

    if (want_stop_) {
        is_reading_ = false;
        return;
    }

    if (received_message_ == nullptr) {
        received_message_ = new message;
    }

    auto buffer = boost::asio::buffer(reinterpret_cast<char *>(received_message_->get_header()),
                                      sizeof(message_header));
    boost::asio::async_read(stream_, buffer,
                            [this](boost::system::error_code ec, size_t bytes_read) {
                                on_received_message_header(ec, bytes_read);
                            });

}

void message_connection_ops::on_received_message_header(boost::system::error_code ec,
                                                        size_t bytes_read) {
    status s;
    if (ec) {
        s.assign(ec.value(), "failed to read message header due to error[" + ec.message() + "]");
        handle_receive_message_error(s);
        return;
    }

    received_message_->validate(s);
    if (!s) {
        handle_receive_message_error(s);
        return;
    }

    if (want_stop_) {
        is_reading_ = false;
        return;
    }

    received_message_->prepare(received_message_->get_body_len());
    char *body = nullptr;
    size_t body_len = 0;
    received_message_->get_body(body, body_len);
    boost::asio::mutable_buffer buffer = boost::asio::buffer(body, body_len);
    boost::asio::async_read(stream_, buffer,
                            [this](boost::system::error_code ec, size_t bytes_read) {
                                on_received_message(ec, bytes_read);
                            });
}

void message_connection_ops::on_received_message(boost::system::error_code ec, size_t bytes_read) {
    status s;
    if (ec) {
        s.assign(ec.value(), "failed to read message body due to error[" + ec.message() + "]");
        handle_receive_message_error(s);
        return;
    }

    if (received_message_->is_request()) {
        handle_request(std::unique_ptr<message>(received_message_), s);
    } else {
        handle_response(std::unique_ptr<message>(received_message_), s);
    }
    received_message_ = nullptr;
    read_message();
}

void message_connection_ops::handle_receive_message_error(const status &s) {
    assert(request_handler_);
    is_reading_ = false;
    request_handler_(nullptr, s);
}

void message_connection_ops::handle_request(std::unique_ptr<message> request, const status &s) {
    assert(request && request->is_request() && request_handler_);
    request_handler_(std::move(request), s);
}

void message_connection_ops::handle_response(std::unique_ptr<message> response, const status &s) {
    assert(response && !response->is_request());
    uint32_t seq = response->get_sequence();
    auto it = wait_response_callbacks_.find(seq);
    if (it == wait_response_callbacks_.cend()) {
        return;
    }
    request_callback_type handler = it->second;
    wait_response_callbacks_.erase(it);
    if (handler) {
        handler(std::move(response), s);
    }
}

void message_connection_ops::do_timer() {
    if (request_deadlines_.empty() || is_timer_running_) {
        is_timer_running_ = false;
        return;
    }

    std::chrono::system_clock::time_point deadline = request_deadlines_.top().deadline;
    timer_.expires_at(deadline);
    timer_.async_wait([this](boost::system::error_code ec) {
        on_timer(ec);
    });
}

void message_connection_ops::on_timer(boost::system::error_code ec) {
    assert(!ec || ec == boost::asio::error::operation_aborted);

    if (ec || want_stop_) {
        is_timer_running_ = false;
        return;
    }

    auto cur = request_deadlines_.top().deadline;
    status s(status::TIMEOUT, "operation timeout");
    while (!request_deadlines_.empty() && cur >= request_deadlines_.top().deadline) {
        const timer_pair &p = request_deadlines_.top();
        on_receive_response(p.request_id, nullptr, s);
        request_deadlines_.pop();
    }
    do_timer();
}

}