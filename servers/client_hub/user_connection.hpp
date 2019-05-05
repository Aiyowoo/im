#ifndef CLIENT_HUB_USER_CONNECTION_H
#define CLIENT_HUB_USER_CONNECTION_H

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
	 * ��������
	 */
	void handle_request(avenue::message* msg);

	void do_squeeze_out();
};

#endif // !CLIENT_HUB_USER_CONNECTION_H
