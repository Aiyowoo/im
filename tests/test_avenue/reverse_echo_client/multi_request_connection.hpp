#ifndef REVERSE_ECHO_CLIENT_MULTI_REQUEST_CONNECTION_H
#define REVERSE_ECHO_CLIENT_MULTI_REQUEST_CONNECTION_H

#include <avenue/client_connection.hpp>

#include <vector>

class multi_request_connection : public avenue::client_connection {
	std::vector<std::string> datas_;
	std::vector<std::string>::const_iterator it_;
public:
	multi_request_connection(boost::asio::io_context& context, boost::asio::ssl::context& ssl_context,
		const std::vector<std::string> &strs);

	void on_initialized(const status& s) override;

	void on_receive_request(avenue::message* msg) override;

	void on_closed() override;

	~multi_request_connection() override;

	void send_info();
};


#endif // !REVERSE_ECHO_CLIENT_MULTI_REQUEST_CONNECTION_H
