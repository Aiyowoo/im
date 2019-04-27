//  main.cpp
// Created by m8792 on 2019/4/21.
// 2019/4/21 15:50

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#endif

#include "reverse_echo_client_connection.hpp"
#include "logger.hpp"
#include "core_dumper.hpp"

#include <comm/status.hpp>
#include <avenue/message.hpp>
#include <avenue/client_connection.hpp>

#include <gtest/gtest.h>

#include <memory>
#include <string>

#include <boost/asio/ssl.hpp>

#include <gtest/gtest.h>

namespace ssl = boost::asio::ssl;

/*
 * 测试能正常连接到服务器，并完成ssl handshake
 */
TEST(reverse_echo_client, can_connect_and_ssl_handshake) {
	try {
		boost::asio::io_context context;
		ssl::context ssl_context(ssl::context::sslv23);
		ssl_context.set_verify_mode(ssl::verify_none);

		/*
		 * 手动tcp连接
		 */
		using tcp = boost::asio::ip::tcp;
		ssl::stream<tcp::socket> stream(context, ssl_context);
		stream.next_layer().async_connect(
			tcp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), 54321),
			[&stream](boost::system::error_code ec) {
				if (ec) {
					ERROR_LOG("failed to connect to server due to error[{}]", ec.message());
					return;
				}
				stream.async_handshake(ssl::stream_base::client,
				                       [&stream](boost::system::error_code ec) {
					                       if (ec) {
						                       ERROR_LOG(
							                       "failed to handshake with server due to error[{}]",
							                       ec.message());
						                       return;
					                       }
					                       DEBUG_LOG("connected");
				                       });
			});
		context.run();
	}
	catch (const std::exception& e) {
		ERROR_LOG("encountered an exception[{}]", e.what());
	}
}

/*
 * 测试单个请求是否工作正常
 */
TEST(reverse_echo_client, one_request) {
	boost::asio::io_context context;
	boost::asio::ssl::context ssl_context(ssl::context::sslv23);
	ssl_context.set_verify_mode(ssl::context::verify_none);
	auto conn_ptr = std::make_shared<reverse_echo_client_connection>(context, ssl_context);
	conn_ptr->run("127.0.0.1", "54321");
	context.run();
}
