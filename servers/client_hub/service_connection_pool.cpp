#include "service_connection_pool.hpp"

static service_connection_pool *pool = nullptr;

void init_service_connection_pool(boost::asio::io_context& context, boost::asio::ssl::context& ssl_context) {
	if(pool == nullptr) {
		pool = new service_connection_pool(context, ssl_context);
	}
}

service_connection_pool& get_service_connection_pool() {
	assert(pool);
	return *pool;
}
