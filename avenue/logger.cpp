//  logger.cpp
// Created by m8792 on 2019/4/21.
// 2019/4/21 22:48

#include "logger.hpp"
#include "details/default_logger.hpp"

namespace avenue {

static default_logger default_logger_;
static log_interface *logger_ = &default_logger_;

void set_logger(log_interface *logger) {
    assert(logger);
    logger_ = logger;
}

log_interface *get_logger() {
    return logger_;
}

}