#ifndef CLIENT_HUB_MESSAGE_CONNECTION_POOL_HPP
#define CLIENT_HUB_MESSAGE_CONNECTION_POOL_HPP

#include "message_connection.hpp"

#include <avenue/connection_pool.hpp>

using message_connection_pool = avenue::connection_pool<message_connection>;

/*
 * ��ʼ����message_connection_pool
 * ֻ�ܵ���һ��
 */
void init_message_connection_pool(boost::asio::io_context &context, boost::asio::ssl::context& ssl_context);

/*
 * ��ȡmessage_connection_pool
 * ���뱣֤init_message_connection_pool�Ѿ�������
 */
message_connection_pool& get_message_connection_pool();

#endif // CLIENT_HUB_MESSAGE_CONNECTION_POOL_HPP
