#ifndef CLIENT_HUB_ERROR_HPP
#define CLIENT_HUB_ERROR_HPP

#include <avenue/message.hpp>

#include <string>

void set_error(avenue::message* msg, int32_t code, const std::string& message);

avenue::message* create_error_message(uint32_t service_id,
                                      uint32_t message_id,
                                      int32_t code,
                                      const std::string& message);

#endif // CLIENT_HUB_ERROR_HPP
