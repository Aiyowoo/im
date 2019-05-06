#include "user_connection.hpp"
#include "user_manager.hpp"
#include "config.hpp"
#include "message_connection_pool.hpp"

#include <servers/constants.hpp>
#include <servers/logger.hpp>
#include <servers/error.hpp>

#include <avenue/message.hpp>

#include <messages/base.pb.h>
#include <messages/user.pb.h>
#include <messages/forward.pb.h>

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

	if (is_squeezed_out_) {
		// 已经被挤掉了
		set_error(msg, status::CONNECTION_SQUEEZED_OUT,
		          fmt::format("user logged in in other device"));
		response(msg);
		return;
	}

	get_server_connection([this, self=shared_from_base(), msg](server_connection_type conn) {
		handle_request(msg, conn);
	});
}

void user_connection::on_closed() {
	DEBUG_LOG("closed");

	if (logged_in_) {
		INFO_LOG("connection closed");
		get_user_manager().remove_connection(user_id_, device_);
		return;
	}
}

void user_connection::squeeze_out() { post([this, self=shared_from_base()] { do_squeeze_out(); }); }

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

bool user_connection::is_logout_request(avenue::message* msg) {
	assert(msg);
	return msg->get_service_id() == im::base::SID_USER &&
		msg->get_message_id() == im::base::MID_LOGOUT;
}

void user_connection::handle_request(avenue::message* msg, server_connection_type conn) {
	if (is_login_request(msg)) {
		handle_login_request(msg, conn);
	}
	else if (is_logout_request(msg)) {
		handle_logout_request(msg, conn);
	}
	else {
		handle_other_request(msg, conn);
	}
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

std::shared_ptr<user_connection> user_connection::shared_from_base() {
	return std::dynamic_pointer_cast<user_connection>(shared_from_this());
}

void user_connection::get_server_connection(query_server_connection_handler handler) {
	const auto wrapped_handler = stream().next_layer().get_io_context().wrap(handler);
	get_message_connection_pool().get_connection(wrapped_handler);
}

void user_connection::get_all_server_connections(query_all_connections_handler handler) {
	const auto wrapped_handler = stream().next_layer().get_io_context().wrap(handler);
	get_message_connection_pool().get_all_connections(wrapped_handler);
}

void user_connection::handle_login_request(avenue::message* msg, server_connection_type conn) {
	assert(msg);
	if (!conn) {
		ERROR_LOG("failed to get message server connection");
		set_error(msg, status::RUNTIME_ERROR,
		          fmt::format("failed to get message server connection"));
		response(msg);
		return;
	}

	const auto service_id = msg->get_service_id();
	const auto message_id = msg->get_message_id();
	const auto seq = msg->get_sequence();
	auto resp_handler = stream().get_io_context().wrap(
		[this, self=shared_from_base(), service_id, message_id, seq](
		avenue::message* msg, const status& s) {
			if (!s) {
				ERROR_LOG("request to message server failed due to error[{}]",
				          s.message());
				assert(!msg);
				msg = create_error_message(service_id, message_id, status::RUNTIME_ERROR,
				                           fmt::format("server internal error"));
				msg->set_sequence(seq);
				response(msg);
				return;
			}

			const char* data = nullptr;
			uint32_t data_len = 0;
			msg->get_body(data, data_len);
			im::user::login_response resp;
			if (resp.ParseFromArray(reinterpret_cast<const void*>(data), data_len)) {
				// 登录成功
				logged_in_ = true;
				user_id_ = resp.info().user_id();
				device_ = resp.device();

				get_user_manager().add_connection(user_id_, device_, shared_from_base());

				msg->set_sequence(seq);
				response(msg);
				return;
			}
			// 出错
			msg->set_sequence(seq);
			response(msg);
		});

	conn->request(msg, resp_handler);
}

void user_connection::handle_logout_request(avenue::message* msg, server_connection_type conn) {
	if(logged_in_) {
		get_user_manager().remove_connection(user_id_, device_);

		is_squeezed_out_ = false;
		logged_in_ = false;
		INFO_LOG("user[{}] device[{}] logged out", user_id_, device);
		user_id_ = INVALID_USER_ID;
		device_ = INVALID_DEVICE;

		close();
	}
}

void user_connection::handle_other_request(avenue::message* msg, server_connection_type conn) {
	// 将其他请求加上target信息后，转发到message_server
	assert(msg);
	if (!conn) {
		ERROR_LOG("failed to get message server connection");
		set_error(msg, status::RUNTIME_ERROR,
			fmt::format("failed to get message server connection"));
		response(msg);
		return;
	}

	const auto service_id = msg->get_service_id();
	const auto message_id = msg->get_message_id();
	const auto seq = msg->get_sequence();

	/*
	 * 根据不同服务转发到不同的服务器
	 */
	const char *original_data = nullptr;
	uint32_t original_data_len = 0;
	msg->get_body(original_data, original_data_len);

	im::forward::forward new_msg;
}
