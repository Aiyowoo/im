//  main.cpp
// Created by m8792 on 2019/4/21.
// 2019/4/21 1:23

#include "reverse_echo_server.hpp"
#include "logger.hpp"

int main() {
    try {
        reverse_echo_server server;
        server.run();
        return 0;
    }
    catch (const std::exception &e) {
        ERROR_LOG("encountered an exception[%s]", e.what());
        return 1;
    }
}