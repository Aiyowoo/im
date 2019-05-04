#ifndef CLIENT_HUB_MESSAGE_CONNECTION_H
#define CLIENT_HUB_MESSAGE_CONNECTION_H

#include "user_manager.hpp"

#include <avenue/client_connection.hpp>

/*
 * ����message_server�������Կͻ��˵�����ת����message_server����
 * ����������message_server������message_server������һ��ֻ��Ҫ���ض���Ϣ���͸�ָ���û�����
 */
class message_connection : avenue::client_connection {
public:
	/*
	 * ��¼����Ļص�
	 */
	using login_handler = std::function<void(avenue::message*)>;

public:
	message_connection(boost::asio::io_context& context, boost::asio::ssl::context& ssl_context);
	~message_connection() = default;

	/*
	 * �����¼
	 */
	void user_login(avenue::message* msg, login_handler handler);

	/*
	 * �ͻ����˳�����Ͽ�����
	 */
	void user_logout(avenue::message* msg, login_handler handler);

	/*
	 * �û�������ת����message_server
	 */
	void user_request(user_manager::user_id_type user_id,
	                  const user_manager::device_type& device,
	                  avenue::message* msg);

	/*
	 * ��������message_server������
	 */
	void on_receive_request(avenue::message* msg) override;
};

#endif // CLIENT_HUB_MESSAGE_CONNECTION_H
