#ifndef AVENUE_TIMER_H
#define AVENUE_TIMER_H

#include <comm/status.hpp>

#include <boost/asio.hpp>

#include <queue>
#include <chrono>
#include <functional>
#include <vector>

namespace avenue {

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

	/*
	 * 在async_wait结束后需要执行的操作
	 * 内部使用，只能包含两种操作：
	 * 1、do_cancel
	 * 2、do_cancel_all
	 */
	using func_type = std::function<void()>;
	std::vector<func_type> todos_;

public:

	explicit timer(boost::asio::io_context& context);
	timer(const timer&) = delete;
	timer& operator=(const timer&) = delete;
	timer(timer&&) = delete;
	timer& operator=(timer&&) = delete;
	/*
	 * 在析构的时候保证将所有的回调到取消掉
	 */
	~timer();

	/*
	 * 等待 d 时间间隔后调用 callback，返回一个timer_id，可以用来取消这次操作，如果操作已经完成，则不能取消
	 * 如果某次等待被取消，则使用status::OPERATION_CANCELLED调用callback
	 */
	timer_id_type wait(clock_type::duration d, const callback_type& callback);

	/*
	 * 取消某次等待操作
	 */
	void cancel(timer_id_type timer_id);

	/*
	 * 取消所有在等待的操作
	 */
	void cancel_all();

	/*
	 * 实际取消某个操作
	 */
	void do_cancel(timer_id_type timer_id);

	/*
	 * 实际取消所有操作
	 */
	void do_cancel_all();

private:
	timer_id_type allocate_timer_id();

	void restart_timing();

	void start_timing();
};

}

#endif // !AVENUE_TIMER_H
