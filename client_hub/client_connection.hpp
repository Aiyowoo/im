#ifndef CLIENT_HUB_CONNECTION_H
#define CLIENT_HUB_CONNECTION_H

#include <avenue/client_connection.hpp>

class client_connection: public avenue::client_connection {
	/*
	 * �û��Ƿ��Ѿ���½
	 */
	bool authenticated_;

	/*
	 * �û�id
	 */
	uint64_t id_;

	/*
	 * �豸id
	 * ��������ͬһ���û��Ĳ�ͬ�豸
	 */
	uint16_t device_id_;

	/*
	 * �û�����ĳ���豸�����ӱ��ߵ�
	 */
	bool kicked_;

	/*
	 * �����Կͻ��˵���֤����
	 */

public:

	client_connection(boost::asio::ip::tcp::socket &socket, boost::asio::ssl::context &ssl_context);

	void on_initialized(const status &s) override;

	/*
	 * �յ����Կͻ��˵���Ϣ���д���
	 */
	void on_receive_request(avenue::message *msg) override;

	/*
	 * �����ӹرպ���connection_manager��ע�������ӣ���ʾ���û��ڸ��豸��������
	 */
	void on_closed() override;

private:
	/*
	 * ��ʼ�ȴ��û���¼������û���ָ��ʱ����û�гɹ���¼����Ͽ�����
	 */
	void wait_user_login();
};

#endif
