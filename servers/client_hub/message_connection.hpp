#ifndef CLIENT_HUB_MESSAGE_CONNECTION_H
#define CLIENT_HUB_MESSAGE_CONNECTION_H

#include "user_manager.hpp"

#include <avenue/client_connection.hpp>

/*
 * 连接message_server，将来自客户端的请求转发到message_server处理，
 * 并处理来自message_server的请求，message_server的请求一般只需要将特定消息发送给指定用户而已
 */
class message_connection : avenue::client_connection {
public:
	/*
	 * 登录请求的回调
	 */
	using login_handler = std::function<void(avenue::message*)>;

public:
	message_connection(boost::asio::io_context& context, boost::asio::ssl::context& ssl_context);
	~message_connection() = default;

	/*
	 * 请求登录
	 */
	void user_login(avenue::message* msg, login_handler handler);

	/*
	 * 客户端退出，或断开连接
	 */
	void user_logout(avenue::message* msg, login_handler handler);

	/*
	 * 用户其他求转发到message_server
	 */
	void user_request(user_manager::user_id_type user_id,
	                  const user_manager::device_type& device,
	                  avenue::message* msg);

	/*
	 * 处理来自message_server的请求
	 */
	void on_receive_request(avenue::message* msg) override;
};

#endif // CLIENT_HUB_MESSAGE_CONNECTION_H
