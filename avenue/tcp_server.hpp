#ifndef AVENUE_TCP_SERVER_H
#define AVENUE_TCP_SERVER_H

#include <boost/asio.hpp>
#include <boost/optional.hpp>
#include <atomic>

#include "listener.hpp"

namespace avenue {

class tcp_server {
    std::vector<boost::asio::io_context*> io_services_;
    std::vector<std::thread> threads_;
    size_t concurrency_count_;
    size_t next_;
    std::vector<boost::asio::ip::tcp::endpoint> listen_addrs_;
    std::vector<listener::connection_handler_type> connection_handlers_;

    std::atomic<bool> is_running_;

public:
    tcp_server();
    ~tcp_server();

    tcp_server& listen(const std::string &ip, uint16_t port, listener::connection_handler_type connection_handler);
    void start(size_t concurrency_count);
    void stop();

    /*
     * 获取主线程的io_context
     */
    boost::asio::io_context& get_main_io_context();

    boost::asio::io_context& get_work_io_context();

private:

    void start_listening();

    void clear();
};

}


#endif //AVENUE_TCP_SERVER_H
