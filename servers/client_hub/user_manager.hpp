#ifndef CLIENT_HUB_USER_MANAGER_H
#define CLIENT_HUB_USER_MANAGER_H

#include <boost/asio.hpp>

#include <unordered_map>
#include <list>

class user_connection;

class user_manager {
public:
	/*
	 * �û�id�����ͣ���ʶ�û�
	 */
	using user_id_type = uint64_t;

	/*
	 * �豸���ͣ����û�ʹ�ò�ͬ�豸��½��ʱ����ܻ��ж�����ӣ�ʹ���豸������
	 */
	using device_type = uint32_t;

	/*
	 * ��������
	 */
	using connection_type = std::shared_ptr<user_connection>;

	struct device_connection_type {
		device_type device;
		connection_type conn;
	};

	using connections_handler_type = std::function<void(const std::list<device_connection_type>&)>;

	using connection_handler_type = std::function<void(const connection_type&)>;

private:

	/*
	 * ʹ��boost asio��ֻ֤��һ���߳̽��в���
	 */
	boost::asio::io_context& context_;

	/*
	 * user_id -> connections
	 */
	std::unordered_map<user_id_type, std::list<device_connection_type>> connections_;

public:
	user_manager(boost::asio::io_context& context);

	/*
	 * �����ƶ��Ϳ���
	 */
	user_manager(const user_manager&) = delete;
	user_manager& operator=(const user_manager&) = delete;
	user_manager(user_manager&&) = delete;
	user_manager& operator=(user_manager&&) = delete;

	/*
	 * fixme: �ر��������ӣ���ȫ�ֶ���ʹ�ã�һ�㲻������
	 */
	~user_manager() = default;

	/*
	 * ���һ������
	 */
	void add_connection(user_id_type user_id, const device_type& device, connection_type connection);

	/*
	 * �Ƴ�һ������
	 */
	void remove_connection(user_id_type user_id, const device_type& device);

	/*
	 * �����û��Ӳ�ͬ�豸�����������е�����
	 */
	void query_connections(user_id_type user_id, connections_handler_type handler);

	/*
	 * �����û���ĳ���ض��豸������������
	 */
	void query_connection(user_id_type user_id, const device_type& device, connection_handler_type handler);

public:
	/*
	 * ֻ���ڲ�����
	 */

	template <typename T>
	void post(T&& t) { context_.post(std::forward<T>(t)); }

	void do_add_connection(user_id_type user_id, const device_type& device, connection_type connection);

	void do_remove_connection(user_id_type user_id, const device_type& device);

	void do_query_connections(user_id_type user_id, connections_handler_type handler);

	void do_query_connection(user_id_type user_id, const device_type &device, connection_handler_type handler);
};

/*
 * ��ʼ����user_manager
 */
void init_user_manager(boost::asio::io_context &context);

/*
 * ��ȡȫ��user_manager
 * ֻ����init_user_manager������֮��ʹ�� 
 */
user_manager& get_user_manager();

#endif // CLIENT_HUB_USER_MANAGER_H
