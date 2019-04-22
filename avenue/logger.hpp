//  logger.hpp
// Created by m8792 on 2019/4/21.
// 2019/4/21 22:48

#ifndef AVENUE_LOGGER_HPP
#define AVENUE_LOGGER_HPP

#include <string>

namespace avenue {

class log_interface {
public:
    virtual void debug_log(const std::string &) = 0;

    virtual void info_log(const std::string &) = 0;

    virtual void warn_log(const std::string &) = 0;

    virtual void error_log(const std::string &) = 0;
};

void set_logger(log_interface *logger);

log_interface *get_logger();

}


#endif //AVENUE_LOGGER_HPP
