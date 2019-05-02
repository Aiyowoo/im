#ifndef CLIENT_HUB_USER_CONNECTION_H
#define CLIENT_HUB_USER_CONNECTION_H

#include <avenue/server_connection.hpp>

/*
 * 来自客户端的请求，处理来自客户端的请求，同时将来自message_server的消息返回给客户端
 */
class user_connection: public avenue::server_connection {
public:
	user_connection(boost::asio::ip::tcp::socket &socket, boost::asio::ssl::context &ssl_context);

	void on_initialized(const status &s) override;

	void on_receive_request(avenue::message *msg) override;

	void on_closed() override;

private:
	void wait_login();
};

#endif // !CLIENT_HUB_USER_CONNECTION_H

