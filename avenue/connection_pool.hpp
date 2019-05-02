#ifndef AVENUE_CONNECTION_POOL_H
#define AVENUE_CONNECTION_POOL_H

#include "client_connection.hpp"
#include "timer.hpp"

#include <boost/asio/ip/tcp.hpp>

namespace avenue {

template <typename ConnectionType>
class connection_pool {
	using address_type = boost::asio::ip::tcp::endpoint;
	/*
	 * 请求池中的连接时，获得了连接后回调函数类型
	 */
	using connection_query_handler = std::function<void(std::shared_ptr<ConnectionType>)>;

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
	std::vector<ConnectionType> connections_;

	/*
	 * 下次请求将返回的连接的偏移
	 */
	size_t next_;

public:
	connection_pool(boost::asio::io_context& context, boost::asio::ssl::context &ssl_context);
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
	 * 形如"127.0.0.1:8081"的数组
	 */
	void connect(const std::vector<std::string>& addresses);

	/*
	 * 指定要连接的服务器的地址
	 */
	void connect(const std::vector<address_type>& addresses);

	/*
	 * 请求一个链接
	 */
	void get_connection(connection_query_handler handler);

	/*
	 * 请求一个链接，如果在timeout时间内没有可用链接，返回nullptr
	 */
	void get_connection(timer::clock_type::duration d, connection_query_handler handler);

	/*
	 * 释放所有链接
	 */
	void close();
};

}

#endif
