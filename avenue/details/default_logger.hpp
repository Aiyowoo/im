//  default_logger.hpp
// Created by m8792 on 2019/4/21.
// 2019/4/21 23:04

#ifndef AVENUE_DEFAULT_LOGGER_HPP
#define AVENUE_DEFAULT_LOGGER_HPP

#include "../logger.hpp"
#include <fmt/printf.h>

namespace avenue {

class default_logger : public log_interface {
public:
    void debug_log(const std::string &string) override {
        fmt::print(string);
    }

    void info_log(const std::string &string) override {
        fmt::print(string);
    }

    void warn_log(const std::string &string) override {
        fmt::print(string);
    }

    void error_log(const std::string &string) override {
        fmt::print(string);
    }
};

}

#endif //AVENUE_DEFAULT_LOGGER_HPP
