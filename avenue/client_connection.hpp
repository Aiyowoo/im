//  client_connection.hpp
// Created by m8792 on 2019/4/24.
// 2019/4/24 22:48

#ifndef AVENUE_CLIENT_CONNECTION_HPP
#define AVENUE_CLIENT_CONNECTION_HPP

#include "message_connection.hpp"

namespace avenue {

class client_connection : public message_connection {

	std::string host_name_;
	std::string service_name_;
	boost::asio::ip::tcp::resolver resolver_;

public:
	client_connection(boost::asio::io_context& context,
	                  boost::asio::ssl::context& ssl_context);

	void run(const std::string& host, const std::string& service);

	std::shared_ptr<client_connection> shared_from_base();

	void on_connected();

	void start_heart_beat();

public:
	/*
	 * 处理在连接和握手时出现的错误
	 * 只能在连接内部调用
	 */
	void handle_initialize_error(const status& error);

#ifdef DEBUG
	/*
	* 调试用信息
	*/

	~client_connection();

	std::string get_extra_log_info() { return ""; }

#endif
};

}


#endif //AVENUE_CLIENT_CONNECTION_HPP
