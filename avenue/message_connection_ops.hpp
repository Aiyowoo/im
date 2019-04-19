//  message_connection.hpp
// Created by m8792 on 2019/4/17.
// 2019/4/17 21:49

#ifndef AVENUE_MESSAGE_CONNECTION_HPP
#define AVENUE_MESSAGE_CONNECTION_HPP

#include "../comm/status.hpp"
#include "message.hpp"

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <functional>
#include <queue>
#include <map>
#include <memory>

namespace avenue {

class message_connection_ops {
public:
    using stream_type = boost::asio::ssl::stream<boost::asio::ip::tcp::socket>;
    using request_callback_type = std::function<void(std::unique_ptr<message> response, const status &)>;
    using request_handler_type = std::function<void(std::unique_ptr<message> request, const status &)>;

private:
    struct timer_pair {
        std::chrono::system_clock::time_point deadline;
        uint32_t request_id;
    };

    struct timer_pair_comp {
        bool operator()(const timer_pair &lhs, const timer_pair &rhs);
    };

    stream_type &stream_;
    boost::asio::system_timer timer_;

    std::list<message *> wait_send_messages_;

    std::priority_queue<timer_pair, std::deque<timer_pair>, timer_pair_comp> request_deadlines_;
    std::map<uint32_t, request_callback_type> wait_response_callbacks_;

    uint64_t tick_count_;

    // 处理收到的请求的函数
    request_handler_type request_handler_;
    message *received_message_;

    /*
     * 控制资源的释放
     */
    bool want_stop_;

    bool is_writing_;

    bool is_reading_;

    bool is_timer_running_;

public:
    explicit message_connection_ops(stream_type &stream);

    ~message_connection_ops();

    void send_request(std::unique_ptr<message> request);

    void send_request(std::unique_ptr<message> request, request_callback_type handler);

    void
    timed_send_request(std::unique_ptr<message> request, uint32_t timeout_seconds,
                       request_callback_type handler);

    /*
     * response丢了没关系，因为重要的消息会在请求时做超时；
     * 如果因为丢掉了response，对端也会超时
     */
    void send_response(std::unique_ptr<message> response);

    /*
     * 开始接受消息
     */
    void start(request_handler_type request_handler);

    /*
     * 停止运行， 并清理资源
     * fixme: 要不要做优雅关闭
     */
    void stop();

    template<typename ...Args>
    void post(Args &&...args) {
        stream_.get_io_context().post(std::forward<Args &&>(args)...);
    }

private:

    void do_send_request(std::unique_ptr<message> request, request_callback_type handler);

    void
    do_timed_send_request(std::unique_ptr<message> request, uint32_t timeout_seconds, request_callback_type handler);

    void do_send_response(std::unique_ptr<message> response);

    void do_start(request_handler_type request_handler);

    void do_stop();

    void start_send_if_need();

    void send_message();

    void on_send_message_header(boost::system::error_code ec, size_t bytes_send);

    void on_send_message_body(boost::system::error_code ec, size_t bytes_send);

    void handle_message_sent(boost::system::error_code ec);

    void on_receive_response(uint32_t request_id, std::unique_ptr<message> response, const status &);

    void read_message();

    void on_received_message_header(boost::system::error_code ec, size_t bytes_read);

    void on_received_message(boost::system::error_code ec, size_t bytes_read);

    void handle_request(std::unique_ptr<message> request, const status &s);

    void handle_response(std::unique_ptr<message> response, const status &s);

    void handle_receive_message_error(const status &s);

    void do_timer();

    void on_timer(boost::system::error_code ec);
};

}


#endif //AVENUE_MESSAGE_CONNECTION_HPP
