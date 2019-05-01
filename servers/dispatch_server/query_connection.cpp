
#include "query_connection.hpp"

#include <messages/base.pb.h>
#include <messages/dispatch.pb.h>

query_connection::query_connection(boost::asio::ip::tcp::socket &socket, 
	boost::asio::ssl::context & ssl_context): avenue::server_connection(socket, ssl_context) {
}

void query_connection::on_initialized(const status& s) {
	send_server_addresses();
}

void query_connection::send_server_addresses() {
	// todo: implement
}
