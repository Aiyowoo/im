//  server_connection.hpp
// Created by m8792 on 2019/4/19.
// 2019/4/19 22:03

#ifndef AVENUE_SERVER_CONNECTION_HPP
#define AVENUE_SERVER_CONNECTION_HPP

#include "message_connection_ops.hpp"
#include <memory>

namespace avenue {

template<typename RequestHandler>
class server_connection : public std::enable_shared_from_this<server_connection<RequestHandler>> {
    using stream_type = boost::asio::ssl::stream<boost::asio::ip::tcp::socket>;
    using started_handler_type = std::function<void(const status &)>;

    stream_type stream_;

    message_connection_ops message_ops_;

    RequestHandler request_handler_;

    boost::asio::system_timer timer_;

public:
    template<typename ...Args>
    server_connection(Args &&...args);

    void start(started_handler_type handler);

    void on_receive_request(std::unique_ptr<message> request, const status &s);

    void request(std::unique_ptr<message> request,
                 message_connection_ops::request_callback_type handler);

    void timed_request(std::unique_ptr<message> request,
                       message_connection_ops::request_handler_type handler, uint32_t timeout_seconds);

    void send_response(std::unique_ptr<message> response);

    void close();

    template<typename ...Args>
    void post(Args &&...args) {
        message_ops_.post(std::forward<Args &&>(args)...);
    }

private:

    void do_start();

    void do_receive_request(std::unique_ptr<message> request, const status &s);

    void do_close();
};

template<typename RequestHandler>
template<typename... Args>
server_connection<RequestHandler>::server_connection(Args &&... args)
        :stream_(std::forward<Args &&>(args)...), message_ops_(stream_), timer_(stream_.get_io_context()) {
}

}

#include "server_connection.ipp"

#endif //AVENUE_SERVER_CONNECTION_HPP
