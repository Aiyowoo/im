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
		std::string host_name; // ��������ַ��ip������
		std::string service_name; // ���������˿ںŻ������
	};

	/*
	 * ������е�����ʱ����������Ӻ�ص���������
	 */
	using connection_query_handler = std::function<void(std::shared_ptr<ConnectionType>)>;

	/*
	 * �������п�������ʱ�Ļص�
	 */
	using all_connections_handler = std::function<void(const std::vector<std::shared_ptr<ConnectionType>>&)>;

	/*
	 * io�����ģ���֤���Ӻ�connection_pool��handler��ִ�У���
	 * ֻ��һ���̵߳��ø�context_.run()ʱ��������ɾ���
	 */
	boost::asio::io_context& context_;

	boost::asio::ssl::context& ssl_context_;

	/*
	 * ��Ҫ���ӵķ�������ַ
	 */
	std::vector<address_type> addresses_;

	/*
	 * ��Ϊ�������̰߳�ȫ�ģ���������������ʱ��������Ҫ�����Ӵ�
	 * ���ӳ����޳�������ֱ�ӷ��ء�
	 */
	std::vector<std::shared_ptr<ConnectionType>> connections_;

	/*
	 * �´����󽫷��ص����ӵ�ƫ��
	 */
	size_t next_;

public:
	connection_pool(boost::asio::io_context& context, boost::asio::ssl::context& ssl_context);
	connection_pool(const connection_pool&) = delete;
	connection_pool& operator=(const connection_pool&) = delete;
	connection_pool(connection_pool&&) = delete;
	connection_pool& operator=(connection_pool&&) = delete;
	/*
	 * fixme: ��Ϊ���ظ��ͻ�����һ��shared_ptr���ͻ�����֮�����ӿ�����Ҫ�ر�
	 * ���Ǵ�Ӧ�ó������������ӳ�һ�㶼���ڳ���������ʱ�򴴽���ֻ���ڳ���ر�ʱ����Ҫ���٣����������û�б�Ҫ
	 */
	~connection_pool() = default;

	/*
	 * ָ��Ҫ���ӵ��ķ������ĵ�ַ
	 * ����"127.0.0.1:8081"�����飬֧������
	 * 
	 * ע�⣺
	 * �ú������ܱ�֤�̰߳�ȫ����Ҫ���÷���֤
	 */
	void connect(const std::vector<std::string>& addresses, status& s);

	/*
	 * ����һ������
	 */
	void get_connection(connection_query_handler handler);

	/*
	 * �������е���������
	 */
	void get_all_connections(all_connections_handler handler);

	/*
	 * todo: ����һ�����ӣ������timeoutʱ����û�п������ӣ�����nullptr
	 */
	// void get_connection(timer::clock_type::duration d, connection_query_handler handler);

	/*
	 * �ͷ���������
	 */
	void close();

public:
	// connection_pool �ڲ�����
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
	// ��next_��ʼ�����������ӣ�ֱ���ҵ�һ�����õ�
	size_t len = connections_.size();
	size_t start = next_;
	std::shared_ptr<ConnectionType> conn;
	do {
		if (!connections_[next_] || !connections_[next_]->running()) {
			// û�����ӻ��������Ѿ��ر�û��������
			connections_[next_] = std::make_shared<ConnectionType>(context_, ssl_context_);
			connections_[next_].run(addresses_[next_].host_name, addresses_[next_].service_name);
		}
		if (connections_[next_]->ok()) {
			// �����Ѿ���ʼ���ɹ�
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
	// �򵥵Ĺر���������
	for (size_t i = 0; i < connections_.size(); ++i) {
		connections_[i]->close();
		connections_[i] = nullptr;
	}
}

}

#endif
