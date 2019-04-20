//
// Created by m8792 on 2019/4/17.
//

#ifndef AVENUE_LISTENER_H
#define AVENUE_LISTENER_H

#include <boost/asio.hpp>
#include <memory>

namespace avenue {

class tcp_server;

class listener : public std::enable_shared_from_this<listener> {
public:
    using connection_handler_type = std::function<void(boost::asio::ip::tcp::socket &)>;

private:
    tcp_server &server_;
    boost::asio::ip::tcp::acceptor acceptor_;
    boost::asio::ip::tcp::endpoint listen_addr_;
    std::shared_ptr<boost::asio::ip::tcp::socket> socket_ptr_;
    connection_handler_type connection_handler_;

public:
    listener(tcp_server &server, const boost::asio::ip::tcp::endpoint &listen_addr, connection_handler_type handler_);

    void start();

    void on_accept(boost::system::error_code ec);
};

}


#endif //AVENUE_LISTENER_H
