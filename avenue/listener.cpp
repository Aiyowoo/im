#include <utility>

//
// Created by m8792 on 2019/4/17.
//

#include "listener.hpp"
#include "tcp_server.hpp"
#include "details/log_helper.hpp"

#include <fmt/format.h>

using tcp = boost::asio::ip::tcp;

namespace avenue {

listener::listener(avenue::tcp_server &server, const boost::asio::ip::tcp::endpoint &listen_addr,
                   connection_handler_type handler_)
        : server_(server), acceptor_(server_.get_main_io_context()),
          listen_addr_(listen_addr), connection_handler_(std::move(handler_)) {}

void listener::start() {
    acceptor_.open(tcp::v4());
    acceptor_.set_option(tcp::acceptor::reuse_address(true));
    acceptor_.bind(listen_addr_);
    acceptor_.listen(tcp::socket::max_listen_connections);

    socket_ptr_ = std::make_shared<tcp::socket>(server_.get_work_io_context());
    acceptor_.async_accept(*socket_ptr_, [this, self = shared_from_this()](boost::system::error_code ec) {
        on_accept(ec);
    });
}

void listener::on_accept(boost::system::error_code ec) {
    if (ec) {
        ERROR_LOG("failed to accept due to error[{}]\n", ec.message());
    } else {
        auto dp = socket_ptr_->remote_endpoint();
        DEBUG_LOG("accept a connection from [{}:{}]", dp.address().to_string(), dp.port());
        connection_handler_(*socket_ptr_);
    }

    socket_ptr_ = std::make_shared<tcp::socket>(server_.get_work_io_context());
    acceptor_.async_accept(*socket_ptr_, [this, self = shared_from_this()](boost::system::error_code ec) {
        on_accept(ec);
    });
}

std::string listener::get_extra_log_info() {
    return fmt::format("listener[{}:{}]", listen_addr_.address().to_string(), listen_addr_.port());
}

}