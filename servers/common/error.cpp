#include "error.hpp"

#include <messages/base.pb.h>

void set_error(avenue::message* msg, int32_t code, const std::string& message) {
	assert(msg);
	im::base::error err;
	err.set_code(code);
	err.set_message(message);
	std::string data;
	err.SerializeToString(&data);
	msg->prepare(data.size());
	char* body = nullptr;
	uint32_t body_len = 0;
	msg->get_body(body, body_len);
	std::copy(data.cbegin(), data.cend(), body);
}

avenue::message* create_error_message(uint32_t service_id,
                                      uint32_t message_id,
                                      int32_t code,
                                      const std::string& message) {
	auto msg = new avenue::message(service_id, message_id);
	set_error(msg, code, message);
	return msg;
}
