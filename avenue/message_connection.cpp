//  message_connection.cpp
// Created by m8792 on 2019/4/21.
// 2019/4/21 23:37

#include "message_connection.hpp"
#include "message.hpp"

#include "details/log_helper.hpp"

#ifdef DEBUG

#include <fmt/format.h>

#endif

namespace asio = boost::asio;

namespace avenue {

message_connection::message_connection(boost::asio::ip::tcp::socket& socket,
                                       boost::asio::ssl::context& ssl_context)
	: stream_(std::move(socket), ssl_context), running_(false), initialized_(false), read_closed_(true),
	  write_closed_(true), want_close_(false), timer_(std::make_shared<timer>(stream_.get_io_context())),
	  recv_message_(nullptr), sequence_(0) {
#ifdef DEBUG
	d_create_time_ = get_current_time_str();
#endif
}

message_connection::message_connection(boost::asio::io_context& io_context,
                                       boost::asio::ssl::context& ssl_context)
	: stream_(io_context, ssl_context), running_(false), initialized_(false), read_closed_(true),
	  write_closed_(true), want_close_(false), timer_(std::make_shared<timer>(stream_.get_io_context())),
	  recv_message_(nullptr), sequence_(0) {
#ifdef DEBUG
	d_create_time_ = get_current_time_str();
#endif
}

message_connection::~message_connection() {
	// 已经被调用了，表明对象已经开始析构，在这里调用close没有意义
	if (recv_message_) {
		delete recv_message_;
		recv_message_ = nullptr;
	}
}

void message_connection::request(message* msg) { request(msg, nullptr); }

void message_connection::request(message* msg, request_callback_type handler) {
	assert(msg);
	post([this, self = shared_from_this(), msg, handler] { do_request(msg, handler); });
}

void message_connection::timed_request(message* msg,
                                       clock_type::duration timeout,
                                       request_callback_type handler) {
	assert(msg);
	clock_type::time_point deadline = clock_type::now() + timeout;
	post([this, self = shared_from_this(), msg, deadline, handler] { do_request(msg, deadline, handler); });
}

void message_connection::response(message* msg) {
	assert(msg);
	post([this, self = shared_from_this(), msg] { do_response(msg); });
}

void message_connection::close() { post([this, self = shared_from_this()] { do_close(); }); }

timer::timer_id_type message_connection::wait(timer::clock_type::duration d, const timer::callback_type& callback) {
	timer::clock_type::time_point invoke_time = clock_type::now() + d;
	return wait(invoke_time, callback);
}

timer::timer_id_type message_connection::
wait(timer::clock_type::time_point time, const timer::callback_type& callback) { return timer_->wait(time, callback); }

bool message_connection::ok() const { return initialized_ && !read_closed_ && !write_closed_ && !want_close_; }

bool message_connection::running() const { return running_; }

void message_connection::set_running(bool running) { running_ = running; }

void message_connection::post(std::function<void()> handler) { stream_.get_io_context().post(handler); }

uint32_t message_connection::allocate_sequence() { return ++sequence_; }

void message_connection::on_request_timeout(uint32_t request_id, status s) {
	if (s.code() == status::OPERATION_CANCELLED) { return; }

	// fixme: 超时返回其他错误
	assert(s);
	auto it = request_callbacks_.find(request_id);
	if (it == request_callbacks_.cend()) { return; }

	auto handler = it->second;
	request_callbacks_.erase(it);
	s.assign(status::TIMEOUT, "request timeout");
	if (handler) { handler(nullptr, s); }
}

void message_connection::do_request(message* msg, request_callback_type handler) {
	assert(msg && msg->is_request());

	DEBUG_LOG("msg[{}]", *msg);

	if (!initialized_ || want_close_ || write_closed_) {
		status s(status::HALF_CLOSED, "write closed or want close write");
		if (handler) { handler(nullptr, s); }
		return;
	}
	uint32_t seq = allocate_sequence();
	msg->set_sequence(seq);
	assert(request_callbacks_.find(seq) == request_callbacks_.cend());
	request_callbacks_.insert({seq, handler});

	send_message(msg);
}

void message_connection::do_request(message* msg,
                                    std::chrono::system_clock::time_point deadline,
                                    message_connection::request_callback_type handler) {
	assert(msg && msg->is_request());

	DEBUG_LOG("msg[{}]", *msg);

	if (!initialized_ || want_close_ || write_closed_) {
		status s(status::HALF_CLOSED, "write closed or want close write");
		handler(nullptr, s);
		return;
	}

	uint32_t seq = allocate_sequence();
	msg->set_sequence(seq);
	assert(request_callbacks_.find(seq) == request_callbacks_.cend());
	request_callbacks_.insert({seq, handler});

	auto timer_id = timer_->wait(deadline, [this, self = shared_from_this(), seq](const status& s) {
		on_request_timeout(seq, s);
	});
	assert(request_id_to_timer_id_.find(seq) == request_id_to_timer_id_.cend());
	request_id_to_timer_id_[seq] = timer_id;

	send_message(msg);
}

void message_connection::do_response(message* msg) {
	assert(msg && !msg->is_request());

	DEBUG_LOG("msg[{}][{}]", reinterpret_cast<void*>(msg), *msg);

	if (!initialized_ || want_close_ || write_closed_) {
		/*
					   * 连接已关闭，不能发送
					   */
		delete msg;
		return;
	}

	send_message(msg);
}

void message_connection::do_close() {
	DEBUG_LOG("connection want to close");
	if (!initialized_) { return; }
	want_close_ = true;
	if (!write_closed_ && waiting_messages_.empty()) {
		boost::system::error_code ec;
		stream_.next_layer().shutdown(asio::ip::tcp::socket::shutdown_send, ec);
		write_closed_ = true;
		if (ec) { ERROR_LOG("failed to close socket write due to error[{}]", ec.message()); }
	}
}

void message_connection::send_message(message* msg) {
	waiting_messages_.emplace_back(msg);
	if (waiting_messages_.size() == 1) {
		// 开始发送
		INFO_LOG("start to send messages...");
		do_send_message();
	}
}

void message_connection::do_send_message() {
	message* msg = nullptr;
	while (msg == nullptr && !waiting_messages_.empty()) {
		msg = waiting_messages_.front();
		assert(msg);

		if (msg->is_request()) {
			auto it = request_callbacks_.find(msg->get_sequence());
			if (it == request_callbacks_.cend()) {
				DEBUG_LOG("msg[{}] already timeout");
				delete msg;
				msg = nullptr;

				waiting_messages_.pop_front();
			}
			else if (!it->second) {
				// don't care response
				request_callbacks_.erase(it);
			}
		}
	}
	if (msg == nullptr) {
		INFO_LOG("no messages need to send, send operation stop...");

		if (want_close_) {
			boost::system::error_code ec;
			stream_.next_layer().shutdown(boost::asio::socket_base::shutdown_send, ec);
			write_closed_ = true;
			INFO_LOG("all messages sent, close write end with result[{}]", ec.message());
		}

		return;
	}

	auto buffer = asio::buffer(reinterpret_cast<const char *>(msg->get_header()),
	                           sizeof(message_header));
	asio::async_write(stream_, buffer, [this, self = shared_from_this()](boost::system::error_code ec, size_t) {
		message* msg = waiting_messages_.front();
		DEBUG_LOG("write message[{}] header, result[{}]", *msg, ec.message());
		if (ec) {
			handle_write_error(ec);
			return;
		}

		const char* data = nullptr;
		uint32_t data_len = 0;
		msg->get_body(data, data_len);
		asio::async_write(stream_, asio::buffer(data, data_len), [this, self](boost::system::error_code ec, size_t) {
			message* msg = waiting_messages_.front();
			DEBUG_LOG("write message[{}] body, result[{}]", *msg, ec.message());
			if (ec) {
				handle_write_error(ec);
				return;
			}

#ifdef DEBUG
			if (msg->is_request()) { ++d_sent_request_count_; }
			else { ++d_sent_response_count_; }
#endif

			delete msg;
			waiting_messages_.pop_front();

			// 继续发送消息
			do_send_message();
		});
	});
}


void message_connection::handle_write_error(boost::system::error_code ec) {
	WARN_LOG("when send messages, encountered an error[{}]", ec.message());
	write_closed_ = true;
	// 将没有发送的消息清理
	status send_error(status::SEND_MESSAGE_ERROR, "write error, can't send message");
	while (!waiting_messages_.empty()) {
		message* msg = waiting_messages_.front();
		waiting_messages_.pop_front();

		// fixme: 只有把这条语句放在外面才能编译通过，否则报未定义的it
		auto it = request_callbacks_.find(msg->get_sequence());
		if (msg->is_request()) {
			// 没有发送出去的请求，直接回调，发送失败
			if (it != request_callbacks_.end() && it->second) { it->second(nullptr, send_error); }
			request_callbacks_.erase(it);
		}
		DEBUG_LOG("failed to send message[{}][{}], due to write error", reinterpret_cast<void *>(msg), *msg);
		delete msg;
	}

	if (read_closed_) {
		// 整个连接都关闭了
		clear_all();
		on_closed();
	}
}

void message_connection::handle_read_error(boost::system::error_code ec) {
	WARN_LOG("when receive message, encountered an error[{}]", ec.message());
	read_closed_ = true;

	// 将所有的request都设置为失败，因为不会再有响应了
	status recv_error(status::RECV_MESSAGE_ERROR, "failed to recv messages");
	for (const auto& p : request_callbacks_) { if (p.second) { p.second(nullptr, recv_error); } }
	request_callbacks_.clear();

	if (write_closed_ || waiting_messages_.empty()) {
		clear_all();
		on_closed();
	}
}

void message_connection::initialize() {
	post([this, self = shared_from_this()] {
		INFO_LOG("do initialize...");
		initialized_ = true;
		read_closed_ = write_closed_ = want_close_ = false;
		start_receiving_message();

		on_initialized(status());


#ifdef DEBUG
		d_initialized_time_ = get_current_time_str();
		auto addr = stream_.next_layer().local_endpoint();
		d_local_addr_ = fmt::format("{}:{}", addr.address().to_string(), addr.port());
		addr = stream_.next_layer().remote_endpoint();
		d_remote_addr_ = fmt::format("{}:{}", addr.address().to_string(), addr.port());
#endif
	});
}

void message_connection::start_receiving_message() {
	if (recv_message_ == nullptr) { recv_message_ = new message; }
	DEBUG_LOG("wait message header...");
	auto buffer = asio::buffer(reinterpret_cast<char *>(recv_message_->get_header()), sizeof(message_header));
	asio::async_read(stream_, buffer, [this, self = shared_from_this()](boost::system::error_code ec, size_t) {
		if (ec) {
			ERROR_LOG("failed to read message header due to error[{}]", ec.message());
			handle_read_error(ec);
			return;
		}

		uint32_t body_len = recv_message_->get_body_len();
		recv_message_->prepare(body_len);
		char* body = nullptr;
		recv_message_->get_body(body, body_len);
		auto buffer = asio::buffer(body, body_len);

		asio::async_read(stream_, buffer, [this, self](boost::system::error_code ec, size_t) {
			if (ec) {
				ERROR_LOG("failed to read message body due to error[{}]", ec.message());
				handle_read_error(ec);
				return;
			}

			handle_received_message(recv_message_);
			recv_message_ = nullptr;
			start_receiving_message();
		});
	});
}

void message_connection::handle_received_message(message* msg) {
	assert(msg);
	if (msg->is_request()) { handle_request(msg); }
	else { handle_response(msg); }

#ifdef DEBUG
	if (msg->is_request()) { ++d_received_request_count_; }
	else { ++d_received_response_count_; }
#endif
}

void message_connection::handle_request(message* msg) {
	// fixme: 当write_close已经关闭的时候是否需要丢弃请求，当前的做法是仍然处理请求
	DEBUG_LOG("received request[{}][{}]", reinterpret_cast<void *>(msg), *msg);
	on_receive_request(msg);
}

void message_connection::handle_response(message* msg) {
	assert(msg);
	DEBUG_LOG("received response[{}][{}]", reinterpret_cast<void *>(msg), *msg);
	uint32_t seq = msg->get_sequence();
	auto it = request_callbacks_.find(seq);
	if (it == request_callbacks_.end()) {
		// 请求已经超时
		delete msg;
		return;
	}

	auto handler = it->second;
	request_callbacks_.erase(it);

	if (handler) { handler(msg, {}); }
	else {
		// 不关心请求结果
		delete msg;
	}

	// 取消计时，如果有的话
	auto timer_id_it = request_id_to_timer_id_.find(seq);
	if (timer_id_it != request_id_to_timer_id_.cend()) {
		timer::timer_id_type timer_id = timer_id_it->second;
		request_id_to_timer_id_.erase(timer_id_it);
		timer_->cancel(timer_id);
	}
}

void avenue::message_connection::clear_all() {
	// 已经停止运行
	set_running(false);

	initialized_ = false;
	boost::system::error_code ec;
	stream_.next_layer().close(ec);
	read_closed_ = true;
	write_closed_ = true;
	timer_->cancel_all();
}

#ifdef DEBUG

std::string message_connection::get_extra_log_info() {
	return fmt::format("connecton[{}] create_time[{}] initialized_time[{}] local_addr[{}] remote_addr[{}] "
	                   "sent_request_count[{}] received_response_count[{}] received_request_count[{}] "
	                   "sent_response_count[{}] timeout_request_count[{}] initialized[{}] "
	                   "read_closed[{}] write_closed[{}] want_close_write[{}] waiting_send_messages_count[{}]",
	                   reinterpret_cast<void*>(this), d_create_time_, d_initialized_time_, d_local_addr_,
	                   d_remote_addr_,
	                   d_sent_request_count_, d_received_response_count_, d_received_request_count_,
	                   d_sent_response_count_, d_timeout_request_count_, initialized_, read_closed_,
	                   write_closed_, want_close_, waiting_messages_.size());
}

#endif

}
