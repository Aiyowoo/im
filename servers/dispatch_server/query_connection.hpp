#ifndef DISPATCH_SERVER
#define DISPATCH_SERVER

#include <avenue/server_connection.hpp>

/*
 * �����Ӵ���ͻ��˵ĵ�������
 */
class query_connection : public avenue::server_connection {
public:
	query_connection(boost::asio::ip::tcp::socket &socket, boost::asio::ssl::context &ssl_context);

	void on_initialized(const status &s) override;

private:
	void send_server_addresses();
};

#endif // !DISPATCH_SERVER
