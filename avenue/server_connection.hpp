//  server_connection.hpp
// Created by m8792 on 2019/4/24.
// 2019/4/24 22:17

#ifndef AVENUE_SERVER_CONNECTION_HPP
#define AVENUE_SERVER_CONNECTION_HPP

#include "message_connection.hpp"

#include <boost/asio/ssl.hpp>

namespace avenue {

class server_connection : public message_connection {

public:
    server_connection(boost::asio::ip::tcp::socket &socket, boost::asio::ssl::context &ssl_context);


    /*
     * 开始ssl handshake -> initialize message_connection
     */
    void run();

    std::shared_ptr<server_connection> shared_from_base();

#ifdef DEBUG

    std::string get_extra_log_info() { return ""; }

#endif

};

}


#endif //AVENUE_SERVER_CONNECTION_HPP
