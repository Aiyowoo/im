#ifndef CLIENT_HUB_USER_MANAGER_H
#define CLIENT_HUB_USER_MANAGER_H

#include <boost/asio.hpp>

#include <unordered_map>
#include <list>

class user_connection;

class user_manager {
public:
	/*
	 * 用户id的类型，标识用户
	 */
	using user_id_type = uint64_t;

	/*
	 * 设备类型，当用户使用不同设备登陆的时候可能会有多个连接，使用设备来区分
	 */
	using device_type = uint32_t;

	/*
	 * 连接类型
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
	 * 使用boost asio保证只有一个线程进行操作
	 */
	boost::asio::io_context& context_;

	/*
	 * user_id -> connections
	 */
	std::unordered_map<user_id_type, std::list<device_connection_type>> connections_;

public:
	user_manager(boost::asio::io_context& context);

	/*
	 * 不能移动和拷贝
	 */
	user_manager(const user_manager&) = delete;
	user_manager& operator=(const user_manager&) = delete;
	user_manager(user_manager&&) = delete;
	user_manager& operator=(user_manager&&) = delete;

	/*
	 * fixme: 关闭所有链接，做全局对象使用，一般不用析构
	 */
	~user_manager() = default;

	/*
	 * 添加一个链接
	 */
	void add_connection(user_id_type user_id, const device_type& device, connection_type connection);

	/*
	 * 移除一个链接
	 */
	void remove_connection(user_id_type user_id, const device_type& device);

	/*
	 * 请求用户从不同设备连上来的所有的链接
	 */
	void query_connections(user_id_type user_id, connections_handler_type handler);

	/*
	 * 请求用户从某个特定设备连上来的连接
	 */
	void query_connection(user_id_type user_id, const device_type& device, connection_handler_type handler);

public:
	/*
	 * 只能内部调用
	 */

	template <typename T>
	void post(T&& t) { context_.post(std::forward<T>(t)); }

	void do_add_connection(user_id_type user_id, const device_type& device, connection_type connection);

	void do_remove_connection(user_id_type user_id, const device_type& device);

	void do_query_connections(user_id_type user_id, connections_handler_type handler);

	void do_query_connection(user_id_type user_id, const device_type &device, connection_handler_type handler);
};

/*
 * 初始化，user_manager
 */
void init_user_manager(boost::asio::io_context &context);

/*
 * 获取全局user_manager
 * 只能在init_user_manager被调用之后使用 
 */
user_manager& get_user_manager();

#endif // CLIENT_HUB_USER_MANAGER_H
