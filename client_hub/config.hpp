#ifndef CLIENT_HUB_CONFIG_H
#define CLIENT_HUB_CONFIG_H

#include <string>
#include <cctype>

#include <comm/status.hpp>

class config_type {
	/*
	 * 客户端连上后，必须在该时间内登录成功，否则自动断开连接
	 * 默认值，为30秒
	 */
	uint32_t login_limited_seconds_ = 30;

public:
	config_type();
	config_type(const config_type&) = delete;
	config_type& operator=(const config_type&) = delete;
	config_type(config_type&&) = delete;
	config_type& operator=(config_type&&) = delete;
	~config_type() = default;

	uint32_t get_login_limited_seconds() const;
	
	void load_config(const status& s);
};

config_type& config();

#endif // !CLIENT_HUB_CONFIG_H
