#ifndef CLIENT_HUB_CONNECTION_H
#define CLIENT_HUB_CONNECTION_H

#include <avenue/client_connection.hpp>

class client_connection: public avenue::client_connection {
	/*
	 * 用户是否已经登陆
	 */
	bool authenticated_;

	/*
	 * 用户id
	 */
	uint64_t id_;

	/*
	 * 设备id
	 * 用来区分同一个用户的不同设备
	 */
	uint16_t device_id_;

	/*
	 * 用户来自某个设备的连接被踢掉
	 */
	bool kicked_;

	/*
	 * 用来对客户端的认证进行
	 */

public:

	client_connection(boost::asio::ip::tcp::socket &socket, boost::asio::ssl::context &ssl_context);

	void on_initialized(const status &s) override;

	/*
	 * 收到来自客户端的消息进行处理
	 */
	void on_receive_request(avenue::message *msg) override;

	/*
	 * 当连接关闭后，在connection_manager中注销该连接，表示该用户在该设备上已下线
	 */
	void on_closed() override;

private:
	/*
	 * 开始等待用户登录，如果用户在指定时间内没有成功登录，则断开连接
	 */
	void wait_user_login();
};

#endif
