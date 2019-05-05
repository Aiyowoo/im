#ifndef AVENUE_CONNECTION_POOL_H
#define AVENUE_CONNECTION_POOL_H

#include "client_connection.hpp"
#include "timer.hpp"

#include <fmt/format.h>

#include <boost/asio/ip/tcp.hpp>

namespace avenue {

template <typename ConnectionType>
class connection_pool {
	struct address_type {
		std::string host_name; // 服务器地址，ip或域名
		std::string service_name; // 服务名，端口号或服务名
	};

	/*
	 * 请求池中的连接时，获得了连接后回调函数类型
	 */
	using connection_query_handler = std::function<void(std::shared_ptr<ConnectionType>)>;

	/*
	 * 请求所有可用连接时的回调
	 */
	using all_connections_handler = std::function<void(const std::vector<std::shared_ptr<ConnectionType>>&)>;

	/*
	 * io上下文，保证连接和connection_pool中handler的执行，在
	 * 只有一个线程调用该context_.run()时，不会造成竞争
	 */
	boost::asio::io_context& context_;

	boost::asio::ssl::context& ssl_context_;

	/*
	 * 需要连接的服务器地址
	 */
	std::vector<address_type> addresses_;

	/*
	 * 因为连接是线程安全的，所以在请求连接时，并不需要将连接从
	 * 连接池中剔除，可以直接返回。
	 */
	std::vector<std::shared_ptr<ConnectionType>> connections_;

	/*
	 * 下次请求将返回的连接的偏移
	 */
	size_t next_;

public:
	connection_pool(boost::asio::io_context& context, boost::asio::ssl::context& ssl_context);
	connection_pool(const connection_pool&) = delete;
	connection_pool& operator=(const connection_pool&) = delete;
	connection_pool(connection_pool&&) = delete;
	connection_pool& operator=(connection_pool&&) = delete;
	/*
	 * fixme: 因为返回给客户的是一个shared_ptr，客户用完之后连接可能需要关闭
	 * 但是从应用场景来看，连接池一般都是在程序启动的时候创建，只有在程序关闭时才需要销毁，所以现如今没有必要
	 */
	~connection_pool() = default;

	/*
	 * 指定要连接到的服务器的地址
	 * 形如"127.0.0.1:8081"的数组，支持域名
	 * 
	 * 注意：
	 * 该函数不能保证线程安全，需要调用方保证
	 */
	void connect(const std::vector<std::string>& addresses, status& s);

	/*
	 * 请求一个链接
	 */
	void get_connection(connection_query_handler handler);

	/*
	 * 请求其中的所有连接
	 */
	void get_all_connections(all_connections_handler handler);

	/*
	 * todo: 请求一个链接，如果在timeout时间内没有可用链接，返回nullptr
	 */
	// void get_connection(timer::clock_type::duration d, connection_query_handler handler);

	/*
	 * 释放所有链接
	 */
	void close();

public:
	// connection_pool 内部调用
	template <typename T>
	void post(T&& t) { context_.post(std::forward<T>(t)); }

	void do_get_connection(connection_query_handler handler);

	void do_get_all_connections(all_connections_handler handler);

	void do_close();
};

template <typename ConnectionType>
connection_pool<ConnectionType>::connection_pool(boost::asio::io_context& context,
                                                 boost::asio::ssl::context& ssl_context): context_(context),
                                                                                          ssl_context_(ssl_context),
                                                                                          next_(0) {
}

template <typename ConnectionType>
void connection_pool<ConnectionType>::connect(const std::vector<std::string>& addresses, status& s) {
	s.clear();
	std::vector<address_type> new_added;
	for (const auto& str : addresses) {
		auto pos = str.find_last_of(':');
		if (pos == std::string::npos) {
			s.assign(status::PARAMETERS_ERROR,
			         fmt::format("format should be host_name:service_name, not {}", str));
			return;
		}
		address_type address;
		address.host_name = str.substr(0, pos);
		address.service_name = str.substr(pos + 1);
		new_added.push_back(address);
	}

	addresses_.insert(addresses_.end(), new_added.cbegin(), new_added.cend());
	connections_.resize(addresses_.size());
}

template <typename ConnectionType>
void connection_pool<ConnectionType>::get_connection(connection_query_handler handler) {
	post([this, handler] { do_get_connection(handler); });
}

template <typename ConnectionType>
void connection_pool<ConnectionType>::get_all_connections(all_connections_handler handler) {
	post([this, handler] {
		do_get_all_connections(handler);
	});
}

template <typename ConnectionType>
void connection_pool<ConnectionType>::close() { post([this] { do_close(); }); }

template <typename ConnectionType>
void connection_pool<ConnectionType>::do_get_connection(connection_query_handler handler) {
	// 从next_开始遍历所有链接，直到找到一个能用的
	size_t len = connections_.size();
	size_t start = next_;
	std::shared_ptr<ConnectionType> conn;
	do {
		if (!connections_[next_] || !connections_[next_]->running()) {
			// 没有连接或者连接已经关闭没有在运行
			connections_[next_] = std::make_shared<ConnectionType>(context_, ssl_context_);
			connections_[next_].run(addresses_[next_].host_name, addresses_[next_].service_name);
		}
		if (connections_[next_]->ok()) {
			// 连接已经初始化成功
			conn = connections_[next_];
		}
		next_ = (next_ + 1) % len;
	}
	while (next_ != start && !conn);

	handler(conn);
}

template <typename ConnectionType>
void connection_pool<ConnectionType>::do_get_all_connections(all_connections_handler handler) {
	std::vector<std::shared_ptr<ConnectionType>> conns;
	for(const auto &conn:connections_) {
		if(conn->ok()) {
			conns.push_back(conn);
		}
	}
	handler(conns);
}

template <typename ConnectionType>
void connection_pool<ConnectionType>::do_close() {
	// 简单的关闭所有链接
	for (size_t i = 0; i < connections_.size(); ++i) {
		connections_[i]->close();
		connections_[i] = nullptr;
	}
}

}

#endif
