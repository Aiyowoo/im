//  message_connection.hpp
// Created by m8792 on 2019/4/21.
// 2019/4/21 23:37

#ifndef AVENUE_MESSAGE_CONNECTION_HPP
#define AVENUE_MESSAGE_CONNECTION_HPP

#include <comm/status.hpp>

#include <memory>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

#include <list>
#include <queue>
#include <map>

#ifdef DEBUG

#include <boost/date_time.hpp>

#endif

namespace avenue {

class message;

class message_connection : public std::enable_shared_from_this<message_connection> {
    using stream_type = boost::asio::ssl::stream<boost::asio::ip::tcp::socket>;
    using request_callback_type = std::function<void(message *, const status &s)>;
    using clock_type = std::chrono::system_clock;

    struct deadline_request_id_p;
    struct deadline_request_id_p_comp;

    using deadline_heap_type =
    std::priority_queue<deadline_request_id_p, std::deque<deadline_request_id_p>, deadline_request_id_p_comp>;

    stream_type stream_;

    /*
     * 建立连接，并完成ssl handshake
     */
    bool initialized_;

    /*
     * 对端关闭了write端，即不能read
     * 这时，对于未发出去的请求，将直接返回connection_closed error
     * 对于未发送出去的响应，仍然尝试发送
     */
    bool read_closed_;

    /*
     * 自己的写端关闭，request(message*, ...)接口将直接返回错误 connection_closed error
     */
    bool write_closed_;

    bool want_close_;

    /* 消息相关 */
    std::list<message *> waiting_messages_;
    std::map<uint32_t, request_callback_type> request_callbacks_;
    struct deadline_request_id_p {
        clock_type::time_point deadline;
        uint32_t request_id;
    };

    struct deadline_request_id_p_comp {
        bool operator()(const deadline_request_id_p &lhs, const deadline_request_id_p &rhs) const;
    };

    deadline_heap_type deadline_heap_;

    boost::asio::system_timer request_timer_;

    /* receive_message */
    message *recv_message_;

public:

    message_connection(boost::asio::ip::tcp::socket &socket, boost::asio::ssl::context &ssl_context);

    message_connection(boost::asio::io_context &io_context, boost::asio::ssl::context &ssl_context);

    /*
     * 都是使用shared_ptr封装后访问，不需要移动
     */
    message_connection(const message_connection &) = delete;

    message_connection &operator=(const message_connection &) = delete;

    message_connection(message_connection &&) = delete;

    message_connection &operator=(message_connection &&) = delete;

    ~message_connection();

    /*
     * 建立建立 -> initialize -> on_initialized
     * 只有在on_initialized被调用之后，才能使用该连接发送消息
     */
    virtual void on_initialized(const status &s);

    /*
     * 收到请求后的处理
     */
    virtual void on_receive_request(message *msg) = 0;

    /*
     * 连接被关闭
     * 连接被完全关闭，所有的消息都已正确处理后调用
     */
    virtual void on_closed() = 0;

    /*
     * 进行初始化
     */
    void initialize();

    /* 消息相关操作 */

    /*
     * 发送请求
     */
    void request(message *msg, request_callback_type handler);

    /*
     * 发送请求， 如果在timeout时间内没有收到response，返回超时
     */
    void timed_request(message *msg, clock_type::duration timeout, request_callback_type handler);

    /*
     * 发送响应
     * 因为如果需要超时功能，可以使用time_request接口来实现
     * 对response多长时间发送没有意义，因为发送了对方不一定能收到，收到了不一定会处理，所以超时需要在request接口上做
     */
    void response(message *msg, request_callback_type handler);

    /*
     * 主动关闭写段，表示不再发送请求
     * 在发送完所有的response之后，关闭socket write
     * 对于对端发送的请求，直接丢弃
     *
     * 想要关闭时，一定要调用close
     */
    void close();

    void post(std::function<void()> handler);

private:

    void do_request(message *msg, request_callback_type handler);

    void do_request(message *msg, clock_type::time_point deadline, request_callback_type handler);

    void do_response(message *msg, request_callback_type handler);

    void do_close();

    /*
     * 发送消息
     * 如果已经在async_write，则直接加入waiting_messages_
     * 如果没有在发送，在加入waiting_messages_，之后开始async_write
     */
    void send_message(message *msg);

    /*
     * 发送waiting_messages_队列中的消息，
     * 直到waiting_messages_为空
     */
    void do_send_message();

    /*
     * 发送消息出错时，进行处理
     * 一般发送消息出错后，就不能在写消息了，
     * 会等待read 也关闭，调用on_close
     */
    void handle_write_error(boost::system::error_code ec);

    /*
     * 读取消息出错时，进行处理
     * 将停止读取消息；如果write并没有close，
     * 则将继续发送完所有的response后，停止
     */
    void handle_read_error(boost::system::error_code ec);

    void start_receiving_message();

    /*
     * 处理收到的消息，可能是response也可能是request
     */
    void handle_received_message(message *msg);

    /*
     * 处理收到的请求
     */
    void handle_request(message *msg);

    /*
     * 处理收到的响应
     */
    void handle_response(message *msg);

    /*
     * 对请求进行计时
     */
    void start_timing();

#ifdef DEBUG
    /*
     * 调试信息
     */
    std::string d_create_time_;
    std::string d_initialized_time_;
    std::string d_local_addr_;
    std::string d_remote_addr_;
    uint32_t d_sent_request_count_ = 0;
    uint32_t d_received_response_count_ = 0;
    uint32_t d_received_request_count_ = 0;
    uint32_t d_sent_response_count_ = 0;
    uint32_t d_timeout_request_count_ = 0;

    std::string get_current_time_str() {
        return boost::posix_time::to_iso_extended_string(boost::posix_time::microsec_clock::local_time());
    }

    std::string get_extra_log_info();

#endif
};

}


#endif //AVENUE_MESSAGE_CONNECTION_HPP
