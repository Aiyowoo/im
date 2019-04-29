#include "timer.hpp"
#include "details/log_helper.hpp"

namespace avenue {

/*
 * 这个版本的实现：
 * 用户使用时，使用用户提供的callbacks来保证timer对象不被析构，
 * 但是当do_cancel_all或cancel_all之后，timer对象中并没有保存任何callback，
 * 此时timer对象可能已经被析构；
 * 即在start_timing的async_wait回调中，do_cancel_all之后继续执行可能崩溃
 */


timer::timer(boost::asio::io_context& context)
: timer_(context), next_timer_id_(1) {}

timer::~timer() {
	// 这时必须保证timer_ 没有进行async_wait操作
	assert(deadline_items_.empty());
}

timer::timer_id_type timer::wait(clock_type::duration d, const callback_type& callback) {
	clock_type::time_point deadline = clock_type::now() + d;
	bool need_restart = (!deadline_items_.empty() && deadline_items_.top().deadline > deadline);
	timer_id_type timer_id = allocate_timer_id();
	assert(callbacks_.find(timer_id) == callbacks_.cend());
	deadline_items_.push(item_type{ deadline, timer_id });
	callbacks_[timer_id] = callback;
	if (need_restart) {
		restart_timing();
	}
	else if (deadline_items_.size() == 1) {
		start_timing();
	}
	return timer_id;
}

void timer::cancel(timer_id_type timer_id) {
	/*
	 * 在一个操作取消后，如果没有其他等待操作，则该timer可能被析构了，
	 * 所以需要注意取消timer async_wait
	 */
	auto it = callbacks_.find(timer_id);
	if (it == callbacks_.cend()) {
		DEBUG_LOG("timer_id[{}] not found, may already finished", timer_id);
		return;
	}

	todos_.push_back([this, timer_id] {
		do_cancel(timer_id);
		});
	if (todos_.size() > 1) {
		// cancel alreay invoked
		return;
	}

	restart_timing();
}

void timer::cancel_all() {
	if (callbacks_.empty()) {
		// nothing need to do
		return;
	}

	bool have_cancelled = !todos_.empty();
	todos_.clear();
	todos_.push_back([this] {
		do_cancel_all();
		});
	if (have_cancelled) {
		// async_wait already cancelled
		return;
	}
	
	restart_timing();
}

void timer::do_cancel(timer_id_type timer_id) {
	auto it = callbacks_.find(timer_id);
	if(it == callbacks_.cend()) {
		return;
	}
	status cancelled(status::OPERATION_CANCELLED, "operation has beed cancelled");
	auto handler = it->second;
	callbacks_.erase(it);
	handler(cancelled);
}

void timer::do_cancel_all() {
	status cancelled(status::OPERATION_CANCELLED, "operation has been cancelled");
	for (const auto& p : callbacks_) {
		auto handler = p.second;
		handler(cancelled);
	}
	callbacks_.clear();
}

timer::timer_id_type timer::allocate_timer_id() {
	return next_timer_id_++;
}


/*
 * 因为该函数被调用时，deadline_items_中的最小值比当前加入的值大，所以timer_一定在async_wait，
 * 不需要考虑启动async_wait
 */
void timer::restart_timing() {
	boost::system::error_code ec;
	timer_.cancel(ec);
	if (ec) {
		ERROR_LOG("timer[{}] failed to cancel timer due to error[{}]", 
			reinterpret_cast<void*>(this), ec.message());
	}
}

void timer::start_timing() {
	bool got_one = false;
	clock_type::time_point deadline;
	while (!deadline_items_.empty()) {
		item_type it = deadline_items_.top();
		if (callbacks_.find(it.timer_id) != callbacks_.cend()) {
			got_one = true;
			deadline = it.deadline;
			break;
		}
		deadline_items_.pop();
	}
	if (!got_one) {
		DEBUG_LOG("timer[{}] nothing need to do...", reinterpret_cast<void*>(this));
		return;
	}

	// fixme: 应该不会抛出异常
	timer_.expires_at(deadline);
	timer_.async_wait([this](boost::system::error_code ec) {
		/*
		 * 执行todos
		 */
		for (auto func : todos_) {
			func();
		}
		todos_.clear();

		if (ec == boost::asio::error::operation_aborted) {
			start_timing();
			return;
		}
		if (ec) {
			ERROR_LOG("timer[{}] encountered an error[{}]", reinterpret_cast<void*>(this), ec.message());
			do_cancel_all();
			return;
		}
			
		// 超时的句柄调用
		status ok;
		clock_type::time_point now = clock_type::now();
		while (!deadline_items_.empty()) {
			item_type item = deadline_items_.top();
			if (item.deadline > now) {
				break;
			}
			deadline_items_.pop();

			auto it = callbacks_.find(item.timer_id);
			if (it != callbacks_.cend()) {
				assert(it->second);
				it->second(ok);
				callbacks_.erase(it);
			}
		}

		start_timing();
	});
}

}