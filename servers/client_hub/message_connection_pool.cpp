#include "message_connection_pool.hpp"

static message_connection_pool *pool = nullptr;

void init_message_connection_pool(boost::asio::io_context& context, boost::asio::ssl::context& ssl_context) {
	if(pool == nullptr) {
		pool = new message_connection_pool(context, ssl_context);
	}
}

message_connection_pool& get_message_connection_pool() {
	assert(pool);
	return *pool;
}
