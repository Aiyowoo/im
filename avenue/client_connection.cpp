//  client_connection.cpp
// Created by m8792 on 2019/4/24.
// 2019/4/24 22:48

#include "client_connection.hpp"
#include "details/log_helper.hpp"

#include "common.hpp"
#include "message.hpp"

namespace asio = boost::asio;
namespace ssl = boost::asio::ssl;
using tcp = asio::ip::tcp;

namespace avenue {

client_connection::client_connection(asio::io_context& context,
                                     ssl::context& ssl_context)
	: message_connection(context, ssl_context), resolver_(context) {
}

void client_connection::run(const std::string& host, const std::string& service) {
	host_name_ = host;
	service_name_ = service;
	DEBUG_LOG("try to connect to [{}:{}]", host_name_, service_name_);

	resolver_.async_resolve(host_name_, service_name_,
	                        [this, self = shared_from_base()](boost::system::error_code ec,
	                                                          tcp::resolver::results_type endpoints) {
		                        if (ec) {
			                        status resolve_error(status::RUNTIME_ERROR,
			                                             fmt::format("failed to resolve {}:{}", host_name_,
			                                                         service_name_));
			                        DEBUG_LOG("failed to resolve {}:{} due to error[{}]",
				                        host_name_, service_name_, ec.message());
			                        on_initialized(resolve_error);
			                        return;
		                        }

		                        asio::async_connect(stream().next_layer(), endpoints,
		                                            [this, self](boost::system::error_code ec,
		                                                         tcp::endpoint ep) {
			                                            if (ec) {
				                                            status connect_error(
					                                            status::CONNECT_FAILED, fmt::format(
						                                            "failed to connect to {}:{} due to error[{}]",
						                                            host_name_, service_name_, ec.message()));
				                                            DEBUG_LOG("{}", connect_error.message());
				                                            on_initialized(connect_error);
				                                            return;
			                                            }

			                                            DEBUG_LOG("client_connection[{}] connect to {}:{} {}:{}", reinterpret_cast<void*>(this),
				                                            host_name_, service_name_,ep.address().to_string(),ep.port()
			                                            );

			                                            on_connected();
		                                            });
	                        });
}

void client_connection::on_connected() {
	stream().async_handshake(ssl::stream_base::client,
	                         [this, self = shared_from_base()](boost::system::error_code ec) {
		                         if (ec) {
			                         status s(ec.value(),
			                                  fmt::format("failed to handshake with server due to error[{}]",
			                                              ec.message()));
			                         on_initialized(s);
			                         return;
		                         }
		                         initialize();
								 start_heart_beat();
	                         });
}

void client_connection::start_heart_beat() {
	message* msg = new avenue::message(HEART_BEAT_SERVICE_ID, HEART_BEAT_MESSAGE_ID, 0);
	timed_request(msg, std::chrono::seconds(HEART_BEAT_TIMEOUT_SECONDS), [this, self = shared_from_base()](message* msg, const status& s) {
		if (!s) {
			ERROR_LOG("connection[{}] heart beat timeout, connection close...", reinterpret_cast<void*>(this));
			close();
			return;
		}
		assert(msg && !msg->is_request);
		start_heart_beat();
	});
}

std::shared_ptr<client_connection> client_connection::shared_from_base() {
	return std::dynamic_pointer_cast<client_connection>(shared_from_this());
}

#ifdef DEBUG

client_connection::~client_connection() {
	DEBUG_LOG("client_connection[{}] destructed", reinterpret_cast<void *>(this));
}

#endif

}
