//  client_connection.cpp
// Created by m8792 on 2019/4/24.
// 2019/4/24 22:48

#include "client_connection.hpp"
#include "details/log_helper.hpp"

namespace asio = boost::asio;
namespace ssl = boost::asio::ssl;
using tcp = asio::ip::tcp;

namespace avenue {

client_connection::client_connection(boost::asio::io_context context,
                                     boost::asio::ssl::context ssl_context)
        : message_connection(context, ssl_context), resolver_(context) {
}

void client_connection::run(const std::string &host, const std::string &service) {
    DEBUG_LOG("try to connect to [{}:{}]", host, service);

    resolver_.async_resolve(host, service,
                            [this, self = shared_from_base()](boost::system::error_code ec,
                                                              tcp::resolver::iterator it) {
                                if (ec) {
                                    status resolve_error(status::RUNTIME_ERROR,
                                                         fmt::format("failed to resolve {}:{}", it->host_name(),
                                                                     it->service_name()));
                                    DEBUG_LOG("failed to resolve {}:{} due to error[{}]",
                                              it->host_name(), it->service_name(), ec.message());
                                    on_initialized(resolve_error);
                                    return;
                                }

                                asio::async_connect(stream().next_layer(), it, tcp::resolver::iterator(),
                                                    [this, self](boost::system::error_code ec,
                                                                 tcp::resolver::iterator it) {
                                                        if (ec) {
                                                            status connect_error(status::CONNECT_FAILED, fmt::format(
                                                                    "failed to connect to {}:{} due to error[{}]",
                                                                    it->host_name(), it->service_name(), ec.message()));
                                                            DEBUG_LOG("{}", connect_error.message());
                                                            on_initialized(connect_error);
                                                            return;
                                                        }

                                                        DEBUG_LOG("client_connection[{}] connect to {}:{} {}:{}",
                                                                  it->host_name(), it->service_name(),
                                                                  it->endpoint().address().to_string(),
                                                                  it->endpoint().port());

                                                        on_connected();
                                                    });
                            });
}

void client_connection::on_connected() {
    stream().async_handshake(ssl::stream_base::client, [this, self = shared_from_base()](boost::system::error_code ec) {
        if (!ec) {
            initialize();
        }
        status s(ec.value(),
                 fmt::format("failed to handshake with server due to error[{}]",
                             ec.message()));
        on_initialized(s);
    });
}


std::shared_ptr<client_connection> client_connection::shared_from_base() {
    return std::dynamic_pointer_cast<client_connection>(shared_from_this());
}

}