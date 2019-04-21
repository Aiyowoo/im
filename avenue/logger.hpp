//
// Created by m8792 on 2019/4/16.
//

#ifndef AVENUE_LOGGER_H
#define AVENUE_LOGGER_H

#include <string.h>
#include <fmt/printf.h>

#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)

#define DEBUG_LOG(format, ...) do { fmt::print("file[{}] line[{}] " format "\n", __FILENAME__, __LINE__, ##__VA_ARGS__);} while(0)
#define INFO_LOG(format, ...) do { fmt::print("file[{}] line[{}] " format "\n", __FILENAME__, __LINE__, ##__VA_ARGS__);} while(0)
#define WARNING_LOG(format, ...) do { fmt::print("file[{}] line[{}] " format "\n", __FILENAME__, __LINE__, ##__VA_ARGS__);} while(0)
#define ERROR_LOG(format, ...) do { fmt::print("file[{}] line[{}] " format "\n", __FILENAME__, __LINE__, ##__VA_ARGS__);} while(0)

#endif //AVENUE_LOGGER_H
