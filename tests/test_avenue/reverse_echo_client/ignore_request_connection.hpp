#ifndef IGNORE_REQUEST_CONNECTION_H
#define IGNORE_REQUEST_CONNECTION_H

#include <avenue/client_connection.hpp>


class ignore_request_connection : public avenue::client_connection {
	size_t send_count_;
	size_t received_count_;
public:
	ignore_request_connection(boost::asio::io_context &context, boost::asio::ssl::context &ssl_context);

	~ignore_request_connection();

	void on_initialized(const status &s) override;

	void on_receive_request(avenue::message* msg) override;

	void on_closed() override;

	void send_a_ignore_request(size_t timeout);
};


#endif // !IGNORE_REQUEST_CONNECTION_H
