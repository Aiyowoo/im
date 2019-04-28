#include "config.hpp"

static const char* const CONFIG_FILE_PATH = "client_config.ini";

config_type::config_type() {
	status s;
	load_config(s);
}

uint32_t config_type::get_login_limited_seconds() const {
	return login_limited_seconds_;
}

void config_type::load_config(const status& s) {
	// TODO: 从配置文件中导入配置
}

config_type& config() {
	static config_type config_;
	return config_;
}
