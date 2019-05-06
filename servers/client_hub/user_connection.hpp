#ifndef CLIENT_HUB_USER_CONNECTION_H
#define CLIENT_HUB_USER_CONNECTION_H

#include "service_connection.hpp"

#include <avenue/server_connection.hpp>

/*
 * 来自客户端的请求，处理来自客户端的请求，同时将来自message_server的消息返回给客户端
 */
class user_connection : public avenue::server_connection {
public:
	/*
	 * 用户id类型
	 */
	using user_id_type = uint64_t;

	/*
	 * 设备类型
	 */
	using device_type = uint32_t;

	/*
	 * 连接到message server的链接类型
	 */
	using server_connection_type = std::shared_ptr<service_connection>;

	/*
	 * 获取到message server的链接后的回调
	 */
	using query_server_connection_handler = std::function<void(server_connection_type)>;

	/*
	 * 获取到所有到message server的链接后的回调
	 */
	using query_all_connections_handler = std::function<void(const std::vector<server_connection_type>&)>;

private:
	/*
	 * 是否已经登录
	 */
	bool logged_in_;

	/*
	 * 用户id
	 */
	user_id_type user_id_;

	/*
	 * 客户端的设备类型
	 */
	device_type device_;

	/*
	 * 客户端ip
	 */
	std::string client_ip_;

	/*
	 * 客户端port
	 */
	uint16_t client_port_;

	/*
	 * 是否被挤掉了
	 */
	bool is_squeezed_out_;

public:
	user_connection(boost::asio::ip::tcp::socket& socket, boost::asio::ssl::context& ssl_context);

	void on_initialized(const status& s) override;

	void on_receive_request(avenue::message* msg) override;

	void on_closed() override;

	/*
	 * 用户相同类型的其他设备上登录，链接被挤掉
	 */
	void squeeze_out();

private:
	/*
	 * 在一定时间内没有登录成功，则关闭连接
	 */
	void wait_login();

	/*
	 * 是否是登录请求
	 */
	static bool is_login_request(avenue::message* msg);

	/*
	 * 是否是退出请求
	 */
	static bool is_logout_request(avenue::message* msg);

	/*
	 * 处理请求
	 */
	void handle_request(avenue::message* msg, server_connection_type conn);

	void do_squeeze_out();

	// ReSharper disable CppHidingFunction
	std::shared_ptr<user_connection> shared_from_base();
	// ReSharper restore CppHidingFunction

	/*
	 * 获取连接到message_server的链接
	 */
	void get_server_connection(query_server_connection_handler handler);

	/*
	 * 获取所有链接到message_server的回调
	 */
	void get_all_server_connections(query_all_connections_handler handler);

	/*
	 * 处理登录请求
	 */
	void handle_login_request(avenue::message *msg, server_connection_type conn);

	/*
	 * 处理退出请求
	 */
	void handle_logout_request(avenue::message *msg, server_connection_type conn);

	/*
	 * 处理其他请求
	 * 加上相关信息后，转发给message_server
	 */
	void handle_other_request(avenue::message *msg, server_connection_type conn);

};

#endif // !CLIENT_HUB_USER_CONNECTION_H
