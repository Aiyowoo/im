#include "multi_request_connection.hpp"
#include "logger.hpp"

#include <avenue/message.hpp>

multi_request_connection::multi_request_connection(boost::asio::io_context & context, boost::asio::ssl::context & ssl_context, 
	const std::vector<std::string>& strs): client_connection(context, ssl_context), datas_(strs), it_(datas_.cbegin()) {
}

void multi_request_connection::on_initialized(const status & s) {
	if(!s) {
		ERROR_LOG("failed to initialize due to error[{}]", s.message());
		return;
	}
	send_info();
}

void multi_request_connection::on_receive_request(avenue::message * msg) {
	assert(false);
	ERROR_LOG("this shouldn't oucrr");
}

void multi_request_connection::on_closed()
{
	DEBUG_LOG("connection[{}] closed", reinterpret_cast<void*>(this));
}

multi_request_connection::~multi_request_connection() {
	DEBUG_LOG("connection[{}] destructed", reinterpret_cast<void*>(this));
}

void multi_request_connection::send_info() {
	if(it_ == datas_.cend()) {
		DEBUG_LOG("all messages sent, over...");
		close();
		return;
	}

	avenue::message *msg = new avenue::message(123, 321, std::distance(datas_.cbegin(), it_));
	msg->set_is_request(true);
	msg->prepare(it_->size());
	char *body = nullptr;
	uint32_t body_len = 0;
	msg->get_body(body, body_len);
	std::copy(it_->cbegin(), it_->cend(), body);
	++it_;
	request(msg, [this, self=shared_from_base()](avenue::message *msg, const status &s) {
		if(!s) {
			ERROR_LOG("failed to send request due to error[{}]", s.message());
			return;
		}
		assert(msg && !msg->is_request());
		const char *body = nullptr;
		uint32_t body_len = 0;
		msg->get_body(body, body_len);
		DEBUG_LOG("connection[{}] got response[{}]", reinterpret_cast<void*>(this), std::string(body, body_len));

		delete msg;

		send_info();
	});
}
