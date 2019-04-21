//  logger.hpp
// Created by m8792 on 2019/4/21.
// 2019/4/21 2:26

#ifndef TEST_LOGGER_HPP
#define TEST_LOGGER_HPP

#include <string.h>
#include <fmt/printf.h>

#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)

#define DEBUG_LOG(format, ...) do { fmt::printf("file[%s] line[%d]" format "\n", __FILENAME__, __LINE__, ##__VA_ARGS__);} while(0)
#define INFO_LOG(format, ...) do { fmt::printf("file[%s] line[%d]" format "\n", __FILENAME__, __LINE__, ##__VA_ARGS__);} while(0)
#define WARNING_LOG(format, ...) do { fmt::printf("file[%s] line[%d]" format "\n", __FILENAME__, __LINE__, ##__VA_ARGS__);} while(0)
#define ERROR_LOG(format, ...) do { fmt::printf("file[%s] line[%d]" format "\n", __FILENAME__, __LINE__, ##__VA_ARGS__);} while(0)


#endif //TEST_LOGGER_HPP
