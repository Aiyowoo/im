#include "user_manager.hpp"
#include "user_connection.hpp"
#include "service_connection_pool.hpp"

#include <servers/logger.hpp>

user_manager::user_manager(boost::asio::io_context& context): context_(context) {
}

void user_manager::add_connection(user_id_type user_id, const device_type& device, connection_type connection) {
	post([this, user_id, device, connection] {
		do_add_connection(user_id, device, connection);
	});
}

void user_manager::remove_connection(user_id_type user_id, const device_type& device) {
	post([this, user_id, device] {
		do_remove_connection(user_id, device);
	});
}

void user_manager::query_connections(user_id_type user_id, connections_handler_type handler) {
	post([this, user_id, handler] {
		do_query_connections(user_id, handler);
	});
}

void user_manager::query_connection(user_id_type user_id, const device_type& device, connection_handler_type handler) {
	post([this, user_id, device, handler] {
		do_query_connection(user_id, device, handler);
	});
}

void user_manager::do_add_connection(user_id_type user_id, const device_type& device, connection_type connection) {
	auto& conns = connections_[user_id];
	auto it = std::find_if(conns.begin(), conns.end(), [&device](const device_connection_type& conn) {
		return conn.device == device;
	});
	if (it != conns.end()) {
		// device类型在该hub上已有连接，需要挤掉
		INFO_LOG("user[{}] from device[{}] two connections, will squeeze out the former", user_id, device);
		it->conn->squeeze_out();
		conns.erase(it);
	}
	else {
		// 需要通知所有所有message server，用户使用某设备连上了该hub
		get_service_connection_pool().get_all_connections(
			[this, user_id, device](const std::vector<std::shared_ptr<service_connection>>& message_conns) {
				for (auto c : message_conns) {
					c->user_connect_notify(user_id, device);
				}
			});
	}

	conns.push_back(device_connection_type{device, connection});
}

void user_manager::do_remove_connection(user_id_type user_id, const device_type& device) {
	auto it = connections_.find(user_id);
	if (it == connections_.cend()) {
		WARN_LOG("no connection found for user_id[{}] device[{}]", user_id, device);
		return;
	}

	auto& conns = it->second;
	const auto conn_it = std::find_if(conns.cbegin(), conns.cend(),
	                                  [&device](const device_connection_type& device_conn) {
		                                  return device == device_conn.device;
	                                  });
	if (conn_it == conns.cend()) {
		WARN_LOG("no connection found for user_id[{}] device[{}]", user_id, device);
		return;
	}

	conns.erase(conn_it);
	if (conns.empty()) {
		// 已经没有user_id的链接了
		connections_.erase(it);
	}
	DEBUG_LOG("removed connection for user[{}] device[{}]", user_id, device);

	// 通知service server连接已经移除
	get_service_connection_pool().get_all_connections(
		[this, user_id, device](const std::vector<std::shared_ptr<service_connection>>& message_conns) {
			for (auto c : message_conns) {
				c->user_disconnect_notify(user_id, device);
			}
		});
}

void user_manager::do_query_connections(user_id_type user_id, connections_handler_type handler) {
	auto it = connections_.find(user_id);
	if (it == connections_.cend()) {
		handler({});
		return;
	}

	assert(!it->second.empty());
	handler(it->second);
}

void user_manager::do_query_connection(user_id_type user_id, const device_type& device,
                                       connection_handler_type handler) {
	const auto it = connections_.find(user_id);
	if (it == connections_.cend()) {
		handler(nullptr);
		return;
	}

	const auto conn_it = std::find_if(it->second.cbegin(), it->second.cend(),
	                                  [&device](const device_connection_type& device_conn) {
		                                  return device_conn.device == device;
	                                  });
	if (conn_it == it->second.cend()) {
		handler(nullptr);
		return;
	}
	handler(conn_it->conn);
}

static user_manager *user_manager_instance = nullptr;

void init_user_manager(boost::asio::io_context& context) {
	if(user_manager_instance == nullptr) {
		user_manager_instance = new user_manager(context);
	}
}

user_manager& get_user_manager() {
	assert(user_manager_instance);
	return *user_manager_instance;
}

