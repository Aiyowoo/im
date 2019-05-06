#ifndef CLIENT_HUB_USER_CONNECTION_H
#define CLIENT_HUB_USER_CONNECTION_H

#include "service_connection.hpp"

#include <avenue/server_connection.hpp>

/*
 * ���Կͻ��˵����󣬴������Կͻ��˵�����ͬʱ������message_server����Ϣ���ظ��ͻ���
 */
class user_connection : public avenue::server_connection {
public:
	/*
	 * �û�id����
	 */
	using user_id_type = uint64_t;

	/*
	 * �豸����
	 */
	using device_type = uint32_t;

	/*
	 * ���ӵ�message server����������
	 */
	using server_connection_type = std::shared_ptr<service_connection>;

	/*
	 * ��ȡ��message server�����Ӻ�Ļص�
	 */
	using query_server_connection_handler = std::function<void(server_connection_type)>;

	/*
	 * ��ȡ�����е�message server�����Ӻ�Ļص�
	 */
	using query_all_connections_handler = std::function<void(const std::vector<server_connection_type>&)>;

private:
	/*
	 * �Ƿ��Ѿ���¼
	 */
	bool logged_in_;

	/*
	 * �û�id
	 */
	user_id_type user_id_;

	/*
	 * �ͻ��˵��豸����
	 */
	device_type device_;

	/*
	 * �ͻ���ip
	 */
	std::string client_ip_;

	/*
	 * �ͻ���port
	 */
	uint16_t client_port_;

	/*
	 * �Ƿ񱻼�����
	 */
	bool is_squeezed_out_;

public:
	user_connection(boost::asio::ip::tcp::socket& socket, boost::asio::ssl::context& ssl_context);

	void on_initialized(const status& s) override;

	void on_receive_request(avenue::message* msg) override;

	void on_closed() override;

	/*
	 * �û���ͬ���͵������豸�ϵ�¼�����ӱ�����
	 */
	void squeeze_out();

private:
	/*
	 * ��һ��ʱ����û�е�¼�ɹ�����ر�����
	 */
	void wait_login();

	/*
	 * �Ƿ��ǵ�¼����
	 */
	static bool is_login_request(avenue::message* msg);

	/*
	 * �Ƿ����˳�����
	 */
	static bool is_logout_request(avenue::message* msg);

	/*
	 * ��������
	 */
	void handle_request(avenue::message* msg, server_connection_type conn);

	void do_squeeze_out();

	// ReSharper disable CppHidingFunction
	std::shared_ptr<user_connection> shared_from_base();
	// ReSharper restore CppHidingFunction

	/*
	 * ��ȡ���ӵ�message_server������
	 */
	void get_server_connection(query_server_connection_handler handler);

	/*
	 * ��ȡ�������ӵ�message_server�Ļص�
	 */
	void get_all_server_connections(query_all_connections_handler handler);

	/*
	 * �����¼����
	 */
	void handle_login_request(avenue::message *msg, server_connection_type conn);

	/*
	 * �����˳�����
	 */
	void handle_logout_request(avenue::message *msg, server_connection_type conn);

	/*
	 * ������������
	 * ���������Ϣ��ת����message_server
	 */
	void handle_other_request(avenue::message *msg, server_connection_type conn);

};

#endif // !CLIENT_HUB_USER_CONNECTION_H
