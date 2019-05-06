#ifndef CLIENT_HUB_MESSAGE_CONNECTION_H
#define CLIENT_HUB_MESSAGE_CONNECTION_H

#include "user_manager.hpp"

#include <avenue/client_connection.hpp>
#include <servers/constants.hpp>

/*
 * ����message_server�������Կͻ��˵�����ת����message_server����
 * ����������message_server������message_server������һ��ֻ��Ҫ���ض���Ϣ���͸�ָ���û�����
 */
class service_connection : public avenue::client_connection {
public:
	/*
	 * ��¼����Ļص�
	 */
	using login_handler = std::function<void(avenue::message*)>;

public:
	service_connection(boost::asio::io_context& context, boost::asio::ssl::context& ssl_context);
	~service_connection() = default;

	void on_initialized(const ::status& s) override;

	void on_closed() override;
	/*
	 * ��������message_server������
	 * �������͸���Ӧ�Ŀͻ��ˣ������������kick user
	 */
	void on_receive_request(avenue::message* msg) override;

	/*
	 * ֪ͨservice_server���û�������hub
	 */
	void user_connect_notify(user_id_type user_id, const device_type& device);

	/*
	 * ֪ͨservice_server�û��˳���ĳ̨�豸�ϵĵ�¼
	 */
	void user_disconnect_notify(user_id_type user_id, const device_type& device);

	/*
	 * �Ƿ����޳��û�������
	 */
	static bool is_kick_device_request(avenue::message* msg);

	/*
	 * �߳�ĳ���û�������
	 */
	void handle_kick_device(avenue::message* msg);
};

#endif // CLIENT_HUB_MESSAGE_CONNECTION_H
