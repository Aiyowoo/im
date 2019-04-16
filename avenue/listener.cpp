#include <utility>

//
// Created by m8792 on 2019/4/17.
//

#include "listener.h"
#include "tcp_server.h"
#include "logger.h"

using tcp = boost::asio::ip::tcp;

namespace avenue {

listener::listener(avenue::tcp_server &server, const boost::asio::ip::tcp::endpoint &listen_addr,
                   connection_handler_type handler_)
        : server_(server), acceptor_(server_.get_main_io_context()),
          listen_addr_(listen_addr), connection_handler_(std::move(handler_)) {}

void listener::start() {
    acceptor_.set_option(tcp::acceptor::reuse_address(true));
    acceptor_.bind(listen_addr_);

    socket_ptr_ = std::make_shared<tcp::socket>(server_.get_work_io_context());
    acceptor_.async_accept(*socket_ptr_, [this, self = shared_from_this()](boost::system::error_code ec) {
        on_accept(ec);
    });
}

void listener::on_accept(boost::system::error_code ec) {
    if (ec) {
        ERROR_LOG("failed to accept due to error[%s]\n", ec.message().c_str());
    } else {
        connection_handler_(*socket_ptr_);
    }

    socket_ptr_ = std::make_shared<tcp::socket>(server_.get_work_io_context());
    acceptor_.async_accept(*socket_ptr_, [this, self = shared_from_this()](boost::system::error_code ec) {
        on_accept(ec);
    });
}

}