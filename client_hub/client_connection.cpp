#include "client_connection.hpp"
#include "logger.hpp"

#define INVALID_USER_ID static_cast<uint64>(-1)
#define INVALID_DEVICE_ID static_cast<uint16_t>(-1)

client_connection::client_connection(boost::asio::ip::tcp::socket& socket,
                                     boost::asio::ssl::context& ssl_context)
: avenue::client_connection(socket, ssl_context), authenticated_(false), id_(INVALID_USER_ID), 
	device_id_(INVALID_DEVICE_ID), kicked_(false) {
}

void client_connection::on_initialized(const status & s)
{
	if(!s) {
		WARN_LOG("connection[{}] failed to initialized with client due to error[{}]",
			reinterpret_cast<void*>(this), s.message());
		return;
	}

	wait_user_login();
}
