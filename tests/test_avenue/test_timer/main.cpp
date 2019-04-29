

#include <avenue/timer.hpp>

#include "logger.hpp"

#include <gtest/gtest.h>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <random>

/*
 * 一个timer，定时一次
 */
TEST(test_timer, once) {
	boost::asio::io_context context;
	{
		auto timer_ptr = std::make_shared<avenue::timer>(context);
		std::string start_time = boost::posix_time::to_iso_extended_string(boost::posix_time::second_clock::local_time());
		timer_ptr->wait(std::chrono::seconds(3), [timer_ptr, start_time](const status &s) {
			std::string end_time = boost::posix_time::to_iso_extended_string(boost::posix_time::second_clock::local_time());
			DEBUG_LOG("timer[{}] start[{}] end[{}]", reinterpret_cast<void*>(&(*timer_ptr)), start_time, end_time);
		});
	}
	context.run();
}

/*
 * 一个timer，定时多次
 */
TEST(test_timer, one_timer_multi_times) {
	boost::asio::io_context context;
	{
		auto timer_ptr = std::make_shared<avenue::timer>(context);
		for(size_t i = 0; i < 10; ++i) {
			using timer = avenue::timer;
			using clock_type = timer::clock_type;
			for (int j = 0; j < 10; ++j) {
				clock_type::time_point expected_end_time = clock_type::now() + std::chrono::seconds(i);
				timer_ptr->wait(std::chrono::seconds(i), [expected_end_time](const status &s) {
					clock_type::time_point now = clock_type::now();
					clock_type::duration d = expected_end_time - now;
					long long second_count =  std::chrono::duration_cast<std::chrono::seconds>(d).count();
					ASSERT_EQ(second_count, 0);
					});
			}
		}
	}
	context.run();
}

/*
 * 多个timer，定时多次
 */
TEST(test_timer, multi_timer_multi_times) {
	boost::asio::io_context context;
	std::random_device rd;
	std::mt19937 gen(rd());
	{
		constexpr int timer_count = 30;
		std::vector<std::shared_ptr<avenue::timer>> timers;
		for(int i = 0; i < timer_count; ++i) {
			timers.emplace_back(new avenue::timer(context));
		}
		std::uniform_int_distribution<> dis(0, timer_count - 1);
		for (size_t i = 0; i < 10; ++i) {
			using timer = avenue::timer;
			using clock_type = timer::clock_type;
			for (int j = 0; j < 100; ++j) {
				auto timer_ptr = timers[dis(gen)];

				clock_type::time_point expected_end_time = clock_type::now() + std::chrono::seconds(i);
				timer_ptr->wait(std::chrono::seconds(i), [expected_end_time](const status &s) {
					clock_type::time_point now = clock_type::now();
					clock_type::duration d = expected_end_time - now;
					long long second_count = std::chrono::duration_cast<std::chrono::seconds>(d).count();
					ASSERT_EQ(second_count, 0);
					});
			}
		}
	}
	context.run();
}
