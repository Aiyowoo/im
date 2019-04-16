//
// Created by m8792 on 2019/4/16.
//

#include "tcp_server.h"
#include "logger.h"
#include "listener.h"

namespace asio = boost::asio;
using tcp = asio::ip::tcp;

namespace avenue {

tcp_server::tcp_server() : concurrency_count_(0), next_(0) {
    is_running_.store(false);
}

tcp_server &avenue::tcp_server::listen(const std::string &ip, uint16_t port,
                                       listener::connection_handler_type connection_handler) {
    listen_addrs_.emplace_back(asio::ip::address::from_string(ip), port);
    connection_handlers_.emplace_back(std::move(connection_handler));
    return *this;
}

void tcp_server::start(size_t concurrency_count) {
    if (concurrency_count == 0) {
        return;
    }

    concurrency_count_ = concurrency_count;
    next_ = 0;
    for (size_t i = 0; i < concurrency_count_; ++i) {
        io_services_.emplace_back(new asio::io_context(1));
    }
    is_running_.store(true);
    for (size_t i = 0; i + 1 < concurrency_count_; ++i) {
        threads_.emplace_back([this, i] {
            asio::io_context &io_ctx = *io_services_[i];
            asio::system_timer timer(io_ctx);
            while (is_running_.load()) {
                timer.expires_from_now(std::chrono::seconds(10));
                timer.async_wait([](boost::system::error_code ec) {
                });
                io_services_[i]->run();
            }
        });
    }

    try {
        start_listening();
    } catch (const std::exception &e) {
        ERROR_LOG("encountered an exception[%s] ...", e.what());
    }

    for (auto &t:threads_) {
        t.join();
    }

    clear();
}

void tcp_server::stop() {
    is_running_.store(false);
    get_main_io_context().stop();
}

void tcp_server::start_listening() {
    auto &io_ctx = get_main_io_context();
    for (size_t i = 0; i < listen_addrs_.size(); ++i) {
        auto listener_ptr = std::make_shared<listener>(*this, listen_addrs_[i], connection_handlers_[i]);
        listener_ptr->start();
    }
    io_ctx.run();
}

boost::asio::io_context &tcp_server::get_main_io_context() {
    return *io_services_.back();
}

boost::asio::io_context &tcp_server::get_work_io_context() {
    if (concurrency_count_ > 1) {
        ++next_;
        return *io_services_[next_ % (concurrency_count_ - 1)];
    }
    return *io_services_.back();
}

tcp_server::~tcp_server() {
    clear();
}

void tcp_server::clear() {
    for (auto &t:threads_) {
        if (t.joinable()) {
            t.join();
        }
    }
    threads_.clear();

    for (auto ptr:io_services_) {
        delete ptr;
    }
    io_services_.clear();

    concurrency_count_ = 0;
    next_ = 0;
}

}