#ifndef CLIENT_HUB_USER_CONNECTION_H
#define CLIENT_HUB_USER_CONNECTION_H

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
	 * 处理请求
	 */
	void handle_request(avenue::message* msg);

	void do_squeeze_out();
};

#endif // !CLIENT_HUB_USER_CONNECTION_H
