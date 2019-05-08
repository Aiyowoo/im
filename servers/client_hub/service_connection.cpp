#include "service_connection.hpp"
#include "user_manager.hpp"
#include "user_connection.hpp"

#include <servers/logger.hpp>
#include <servers/error.hpp>

#include <avenue/message.hpp>
#include <messages/forward.pb.h>

service_connection::service_connection(boost::asio::io_context& context, boost::asio::ssl::context& ssl_context)
	: avenue::client_connection(context, ssl_context) {
}

void service_connection::on_initialized(const ::status& s) {
	if (!s) {
		ERROR_LOG("failed to connect to service server due to error[{}]", s.message());
		return;
	}
	INFO_LOG("connect to service server successfully");
}

void service_connection::on_closed() {
	INFO_LOG("service connection closedf");
}

void service_connection::on_receive_request(avenue::message* msg) {
	// 提取出其中的消息，发送给对象的用户
	assert(msg);

	// TODO: 处理剔除客户端的消息
	if(is_kick_device_request(msg)) {
		handle_kick_device(msg);
		return;
	}

	const char* data = nullptr;
	uint32_t data_len = 0;
	msg->get_body(data, data_len);
	im::forward::forward f;
	f.ParseFromArray(data, data_len);
	const int target_len = f.targets_size();

	const auto service_id = msg->get_service_id();
	const auto message_id = msg->get_message_id();
	const auto seq = msg->get_sequence();

	auto handler = stream().get_io_context().wrap(
		[this, self = shared_from_base(), service_id, message_id, seq](
		avenue::message* msg,
		const status& s) {
			// 将客户端的响应转发到service server
			if (!s) {
				ERROR_LOG("failed to request to client due to error[{}]", s.message());
				assert(!msg);
				msg = create_error_message(service_id, message_id, status::RUNTIME_ERROR,
				                           "server internal error");
				msg->set_sequence(seq);
				response(msg);
				return;
			}

			msg->set_sequence(seq);
			response(msg);
		});
	for (int i = 0; i < target_len; ++i) {
		auto* new_msg = new avenue::message(msg->get_service_id(), msg->get_message_id());
		new_msg->set_body(f.data());

		auto& target = f.targets(i);
		if (target.device() == im::base::client_type::ALL) {
			// 转发给所有客户端
			get_user_manager().query_connections(target.user_id(),
			                                     [this, self = shared_from_base(), new_msg, handler](
			                                     const std::list<user_manager::device_connection_type>& conns) {
				                                     for (auto& device_conn : conns) {
					                                     device_conn.conn->request(new_msg, handler);
				                                     }
			                                     });
		}
		else {
			// 转发给其中一个客户端
			get_user_manager().query_connection(target.user_id(), target.device(),
			                                    [this, self = shared_from_this(), new_msg, handler](
			                                    user_manager::connection_type conn) {
				                                    conn->request(new_msg, handler);
			                                    });
		}
	}
}

void service_connection::user_connect_notify(user_id_type user_id, const device_type& device) {
	im::forward::user_logged_in req;
	req.set_user_id(user_id);
	req.set_device(static_cast<im::base::client_type>(device));
	std::string data;
	req.SerializeToString(&data);
	auto msg = new avenue::message(im::base::SID_LOGIN, im::base::login_messages::USER_LOGGED_IN);
	msg->set_body(data);
	request(msg);
}

void service_connection::user_disconnect_notify(user_id_type user_id, const device_type& device) {
	im::forward::user_logged_out req;
	req.set_user_id(user_id);
	req.set_device(static_cast<im::base::client_type>(device));
	std::string data;
	req.SerializeToString(&data);
	auto msg = new avenue::message(im::base::SID_LOGIN, im::base::USER_LOGGED_OUT);
	msg->set_body(data);
	request(msg);
}

bool service_connection::is_kick_device_request(avenue::message* msg) {
	assert(msg);
	return msg->get_service_id() == im::base::SID_USER &&
		msg->get_message_id() == im::base::MID_KICK_DEVICE;
}

void service_connection::handle_kick_device(avenue::message* msg) {
	assert(msg);
	const char* data = nullptr;
	size_t data_len = 0;
	msg->get_body(data, data_len);

	im::forward::forward f;
	if(!f.ParseFromArray(data, data_len)) {
		ERROR_LOG("failed to parse kick user message");
		set_error(msg, status::PARAMETERS_ERROR, "failed to parse message");
		response(msg);
		return;
	}

	assert(f.targets_size() == 1);
	// TODO: implementation
}
