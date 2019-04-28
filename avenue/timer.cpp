#include "timer.hpp"
#include "details/log_helper.hpp"

namespace avenue {

timer::timer(boost::asio::io_context& context)
: timer_(context), next_timer_id_(1) {}

timer::~timer() {
	// ��ʱ���뱣֤timer_ û�н���async_wait����
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
	auto it = callbacks_.find(timer_id);
	if (it == callbacks_.cend()) {
		DEBUG_LOG("timer[{}] id[{}] already finished", reinterpret_cast<void*>(this), timer_id);
		return;
	}

	auto handler = it->second;
	callbacks_.erase(it);
	status cancelled(status::OPERATION_CANCELLED, "operation has been cancelled");
	handler(cancelled);
}

void timer::cancel_all() {
	status cancelled(status::OPERATION_CANCELLED, "operation has been cancelled");
	for (const auto& p : callbacks_) {
		p.second(cancelled);
	}
	cancelled.clear();
	boost::system::error_code ec;
	timer_.cancel(ec);
}

timer::timer_id_type timer::allocate_timer_id() {
	return next_timer_id_++;
}


/*
 * ��Ϊ�ú���������ʱ��deadline_items_�е���Сֵ�ȵ�ǰ�����ֵ������timer_һ����async_wait��
 * ����Ҫ��������async_wait
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

	// fixme: Ӧ�ò����׳�����
	timer_.expires_at(deadline);
	timer_.async_wait([this](boost::system::error_code ec) {
		if (ec == boost::asio::error::operation_aborted) {
			start_timing();
			return;
		}
		if (ec) {
			ERROR_LOG("timer[{}] encountered an error[{}]", reinterpret_cast<void*>(this), ec.message());
			cancel_all();
			return;
		}
			
		// ��ʱ�ľ������
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