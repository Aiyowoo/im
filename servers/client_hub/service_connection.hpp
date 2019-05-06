#ifndef CLIENT_HUB_MESSAGE_CONNECTION_H
#define CLIENT_HUB_MESSAGE_CONNECTION_H

#include "user_manager.hpp"

#include <avenue/client_connection.hpp>
#include <servers/constants.hpp>

/*
 * 连接message_server，将来自客户端的请求转发到message_server处理，
 * 并处理来自message_server的请求，message_server的请求一般只需要将特定消息发送给指定用户而已
 */
class service_connection : public avenue::client_connection {
public:
	/*
	 * 登录请求的回调
	 */
	using login_handler = std::function<void(avenue::message*)>;

public:
	service_connection(boost::asio::io_context& context, boost::asio::ssl::context& ssl_context);
	~service_connection() = default;

	void on_initialized(const ::status& s) override;

	void on_closed() override;
	/*
	 * 处理来自message_server的请求
	 * 将请求发送给对应的客户端，处理特殊的如kick user
	 */
	void on_receive_request(avenue::message* msg) override;

	/*
	 * 通知service_server有用户连上了hub
	 */
	void user_connect_notify(user_id_type user_id, const device_type& device);

	/*
	 * 通知service_server用户退出了某台设备上的登录
	 */
	void user_disconnect_notify(user_id_type user_id, const device_type& device);

	/*
	 * 是否是剔除用户的请求
	 */
	static bool is_kick_device_request(avenue::message* msg);

	/*
	 * 踢出某个用户的连接
	 */
	void handle_kick_device(avenue::message* msg);
};

#endif // CLIENT_HUB_MESSAGE_CONNECTION_H
