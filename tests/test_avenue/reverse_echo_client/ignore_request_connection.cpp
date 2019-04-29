#include "ignore_request_connection.hpp"
#include "logger.hpp"

#include <avenue/message.hpp>

#include <boost/date_time/posix_time/posix_time.hpp>

constexpr uint32_t IGNORE_SERVICE_ID = 302;
constexpr uint32_t IGNORE_MESSAGE_ID = 302;

constexpr uint32_t total_count = 10;


ignore_request_connection::ignore_request_connection(boost::asio::io_context& context, boost::asio::ssl::context& ssl_context)
: avenue::client_connection(context, ssl_context), send_count_(0), received_count_(0) {
	DEBUG_LOG("ignore_request_connection[{}] constructed", reinterpret_cast<void*>(this));
}

ignore_request_connection::~ignore_request_connection() {
	DEBUG_LOG("ignore_request_connection[{}] destructed");
}

void ignore_request_connection::on_initialized(const status& s) {
	DEBUG_LOG("ignore_request_connection[{}] initialized with status code[{}] message[{}]",
		reinterpret_cast<void*>(this), s.code(), s.message());
	
	if (!s) {
		ERROR_LOG("failed to initialized due to error[{}]", s.message());
		return;
	}
	
	for (int i = 0; i < total_count; ++i) {
		send_a_ignore_request(i);
		++send_count_;
	}
}

void ignore_request_connection::on_receive_request(avenue::message* msg) {
	throw std::exception("not implement");
}

void ignore_request_connection::on_closed() {
	DEBUG_LOG("ignore_request_connection[{}] closed...", reinterpret_cast<void*>(this));
}

void ignore_request_connection::send_a_ignore_request(size_t timeout) {
	avenue::message *msg = new avenue::message(IGNORE_SERVICE_ID, IGNORE_MESSAGE_ID);
	msg->set_is_request(true);
	std::string start_time = boost::posix_time::to_iso_string(boost::posix_time::microsec_clock::local_time());
	timed_request(msg, std::chrono::seconds(timeout), [this, self = shared_from_this(), start_time, timeout](avenue::message* resp, const status& s) {
		std::string end_time = boost::posix_time::to_iso_string(boost::posix_time::microsec_clock::local_time());

		DEBUG_LOG("ignore_request_connection[{}] start time[{}] end time[{}] timeout[{}]",
			reinterpret_cast<void*>(this), start_time, end_time, timeout);
		assert(s.code() == status::TIMEOUT);
		assert(!resp);
		++received_count_;
		if (received_count_ == send_count_) {
			close();
		}
	});
}
