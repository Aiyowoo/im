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
	 * ������е�����ʱ����������Ӻ�ص���������
	 */
	using connection_query_handler = std::function<void(std::shared_ptr<ConnectionType>)>;

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
	std::vector<ConnectionType> connections_;

	/*
	 * �´����󽫷��ص����ӵ�ƫ��
	 */
	size_t next_;

public:
	connection_pool(boost::asio::io_context& context, boost::asio::ssl::context &ssl_context);
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
	 * ����"127.0.0.1:8081"������
	 */
	void connect(const std::vector<std::string>& addresses);

	/*
	 * ָ��Ҫ���ӵķ������ĵ�ַ
	 */
	void connect(const std::vector<address_type>& addresses);

	/*
	 * ����һ������
	 */
	void get_connection(connection_query_handler handler);

	/*
	 * ����һ�����ӣ������timeoutʱ����û�п������ӣ�����nullptr
	 */
	void get_connection(timer::clock_type::duration d, connection_query_handler handler);

	/*
	 * �ͷ���������
	 */
	void close();
};

}

#endif
