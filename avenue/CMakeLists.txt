
CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

PROJECT(avenue)

FIND_PACKAGE(fmt CONFIG REQUIRED)

FIND_PACKAGE(Boost REQUIRED)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})

INCLUDE_DIRECTORIES($ENV{IM_INSTALL_PATH}/include)
LINK_DIRECTORIES($ENV{IM_INSTALL_PATH}/lib/comm)

ADD_LIBRARY(avenue message.cpp message.hpp tcp_server.cpp tcp_server.hpp logger.cpp logger.hpp
            listener.cpp listener.hpp message_connection_ops.cpp
            message_connection_ops.hpp server_connection.ipp server_connection.hpp
            client_connection.cpp client_connection.hpp)
TARGET_LINK_LIBRARIES(avenue comm fmt::fmt fmt::fmt-header-only)

INSTALL(TARGETS avenue DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/avenue)
INSTALL(FILES tcp_server.hpp server_connection.hpp message.hpp listener.hpp client_connection.hpp
        message_connection_ops.hpp server_connection.ipp logger.hpp
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include/avenue)