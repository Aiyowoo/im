#ifndef CLIENT_HUB_MESSAGE_CONNECTION_POOL_HPP
#define CLIENT_HUB_MESSAGE_CONNECTION_POOL_HPP

#include "message_connection.hpp"

#include <avenue/connection_pool.hpp>

using message_connection_pool = avenue::connection_pool<message_connection>;

/*
 * 初始化，message_connection_pool
 * 只能调用一次
 */
void init_message_connection_pool(boost::asio::io_context &context, boost::asio::ssl::context& ssl_context);

/*
 * 获取message_connection_pool
 * 必须保证init_message_connection_pool已经被调用
 */
message_connection_pool& get_message_connection_pool();

#endif // CLIENT_HUB_MESSAGE_CONNECTION_POOL_HPP
