//  log_helper.hpp
// Created by m8792 on 2019/4/21.
// 2019/4/21 23:11

#ifndef AVENUE_LOG_HELPER_HPP
#define AVENUE_LOG_HELPER_HPP

#include "../logger.hpp"
#include <string>
#include <fmt/format.h>

inline std::string get_extra_log_info() {
    return "";
}

#define DEBUG_LOG(f, ...) do {avenue::get_logger()->debug_log(get_extra_log_info() + " " + fmt::format(f "\n", ##__VA_ARGS__));} while(0)
#define INFO_LOG(f, ...) do {avenue::get_logger()->info_log(get_extra_log_info() + " " + fmt::format(f "\n", ##__VA_ARGS__));} while(0)
#define WARN_LOG(f, ...) do {avenue::get_logger()->warn_log(get_extra_log_info() + " " + fmt::format(f "\n", ##__VA_ARGS__));} while(0)
#define ERROR_LOG(f, ...) do {avenue::get_logger()->error_log(get_extra_log_info() + " " + fmt::format(f "\n", ##__VA_ARGS__));} while(0)

#endif //AVENUE_LOG_HELPER_HPP
