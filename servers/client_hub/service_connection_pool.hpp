#ifndef CLIENT_HUB_SERVICE_CONNECTION_POOL_HPP
#define CLIENT_HUB_SERVICE_CONNECTION_POOL_HPP

#include "service_connection.hpp"

#include <avenue/connection_pool.hpp>

using service_connection_pool = avenue::connection_pool<service_connection>;

/*
 * ��ʼ����message_connection_pool
 * ֻ�ܵ���һ��
 */
void init_service_connection_pool(boost::asio::io_context &context, boost::asio::ssl::context& ssl_context);

/*
 * ��ȡmessage_connection_pool
 * ���뱣֤init_message_connection_pool�Ѿ�������
 */
service_connection_pool& get_service_connection_pool();

#endif // CLIENT_HUB_SERVICE_CONNECTION_POOL_HPP
