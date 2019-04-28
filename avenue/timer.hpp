#ifndef AVENUE_TIMER_H
#define AVENUE_TIMER_H

#include <comm/status.hpp>

#include <boost/asio.hpp>

#include <queue>
#include <chrono>
#include <functional>

namespace avenue {

/*
 * ʹ��ʱ���豣֤��timer����֮ǰ�����еĻص����ѽ���
 * ��Ϊ����֪����ʲôʱ��timer�Ļص��ᱻ����
 * ����һ����async_waitʱ������û�б�����
 */
class timer {
public:
	using timer_id_type = uint32_t;
	using clock_type = std::chrono::system_clock;
	using callback_type = std::function<void(const status&)>;

private:
	struct item_type {
		timer::clock_type::time_point deadline;
		timer::timer_id_type timer_id;
	};

	struct item_comp {
		bool operator()(const item_type& lhs, const item_type& rhs) const {
			return lhs.deadline > rhs.deadline;
		}
	};
private:
	boost::asio::system_timer timer_;

	std::priority_queue<item_type, std::deque<item_type>, item_comp> deadline_items_;

	std::map<timer_id_type, callback_type> callbacks_;

	timer_id_type next_timer_id_;

public:

	explicit timer(boost::asio::io_context& context);
	timer(const timer&) = delete;
	timer& operator=(const timer&) = delete;
	timer(timer&&) = delete;
	timer& operator=(timer&&) = delete;
	/*
	 * ��������ʱ��֤�����еĻص���ȡ����
	 */
	~timer();

	/*
	 * �ȴ� d ʱ��������� callback������һ��timer_id����������ȡ����β�������������Ѿ���ɣ�����ȡ��
	 * ���ĳ�εȴ���ȡ������ʹ��status::OPERATION_CANCELLED����callback
	 */
	timer_id_type wait(clock_type::duration d, const callback_type& callback);

	/*
	 * ȡ��ĳ�εȴ�����
	 */
	void cancel(timer_id_type timer_id);

	/*
	 * ȡ�������ڵȴ��Ĳ���
	 */
	void cancel_all();

private:
	timer_id_type allocate_timer_id();

	void restart_timing();

	void start_timing();
};

}

#endif // !AVENUE_TIMER_H
