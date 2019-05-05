#include "user_connection.hpp"
#include "user_manager.hpp"
#include "config.hpp"

#include <servers/constants.hpp>
#include <servers/logger.hpp>

#include <avenue/message.hpp>

#include <messages/base.pb.h>
#include <messages/user.pb.h>

user_connection::user_connection(boost::asio::ip::tcp::socket& socket, boost::asio::ssl::context& ssl_context)
	: avenue::server_connection(socket, ssl_context), logged_in_(false), user_id_(INVALID_USER_ID),
	  device_(INVALID_DEVICE), client_port_(0), is_squeezed_out_(false) {
	// socket 已经连接好了，可以直接获取客户端ip，port
	boost::system::error_code ec;
	const auto ep = stream().next_layer().remote_endpoint(ec);
	if (ec) {
		ERROR_LOG("failed to get client socket address due to error[{}]", ec.message());
		return;
	}
	client_ip_ = ep.address().to_string();
	client_port_ = ep.port();
}

void user_connection::on_initialized(const status& s) {
	if (!s) {
		ERROR_LOG("initialize failed due to error[{}]")
		return;
	}

	wait_login();
}

void user_connection::on_receive_request(avenue::message* msg) {
	if (!logged_in_ && !is_login_request(msg)) {
		// 只有登录后才能发送其他请求，非法请求，关闭连接
		ERROR_LOG("not logged in, received request service_id[{}] message_id[{}]",
		          msg->get_service_id(), msg->get_message_id());
		close();
		return;
	}

	handle_request(msg);
}

void user_connection::on_closed() {
	DEBUG_LOG("closed");

	if (logged_in_) {
		INFO_LOG("connection closed");
		get_user_manager().remove_connection(user_id_, device_);
		return;
	}
}

void user_connection::squeeze_out() {
	post([this, self=shared_from_base()] {
		do_squeeze_out();
	});
}

void user_connection::wait_login() {
	wait(std::chrono::seconds(config().get_login_limited_seconds()),
	     [this, self=shared_from_base()](const status&) {
		     if (!logged_in_) {
			     WARN_LOG("user not login in {} seconds, close connection...", config().get_login_limited_seconds());
			     close();
		     }
	     });
}

bool user_connection::is_login_request(avenue::message* msg) {
	assert(msg);
	return msg->get_service_id() == im::base::SID_USER &&
		msg->get_message_id() == im::base::MID_LOGIN;
}

void user_connection::handle_request(avenue::message* msg) {
	// TODO: 处理来自客户端的各种请求
}

void user_connection::do_squeeze_out() {
	// 发送被挤掉消息后，关闭连接
	is_squeezed_out_ = true;
	im::user::kick_user_request request;
	request.set_user_id(user_id_);
	request.set_reason(im::user::kick_reason::KICK_BY_OTHER_DEVICE);
	std::string data;
	if (!request.SerializeToString(&data)) {
		CRITICAL_LOG("failed to serialize protobuf message");
		close();
		return;
	}

	auto msg = new avenue::message(im::base::services::SID_USER, im::base::user_messages::MID_KICK_DEVICE);
	msg->prepare(data.size());
	char* body = nullptr;
	size_t body_len = 0;
	msg->get_body(body, body_len);
	std::copy(data.cbegin(), data.cend(), body);

	timed_request(msg, USER_REQUEST_TIMEOUT, [this, self=shared_from_base()](avenue::message* resp, const status& s) {
		delete resp;
		if (!s) {
			ERROR_LOG("failed to send squeeze out request to client due to error[{}]", s.message());
			return;
		}
		close();
	});
}
