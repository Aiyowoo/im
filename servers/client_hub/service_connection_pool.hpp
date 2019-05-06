#ifndef CLIENT_HUB_SERVICE_CONNECTION_POOL_HPP
#define CLIENT_HUB_SERVICE_CONNECTION_POOL_HPP

#include "service_connection.hpp"

#include <avenue/connection_pool.hpp>

using service_connection_pool = avenue::connection_pool<service_connection>;

/*
 * 初始化，message_connection_pool
 * 只能调用一次
 */
void init_service_connection_pool(boost::asio::io_context &context, boost::asio::ssl::context& ssl_context);

/*
 * 获取message_connection_pool
 * 必须保证init_message_connection_pool已经被调用
 */
service_connection_pool& get_service_connection_pool();

#endif // CLIENT_HUB_SERVICE_CONNECTION_POOL_HPP
