
CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

PROJECT(avenue)

FIND_PACKAGE(Boost REQUIRED)
IF (NOT Boost_FOUND)
    MESSAGE(FATAL_ERROR "failed to load boost")
ELSE (NOT Boost_FOUND)
    INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
ENDIF (NOT Boost_FOUND)

ADD_LIBRARY(avenue message.cpp message.hpp tcp_server.cpp tcp_server.h logger.cpp logger.h ../comm/status.cpp ../comm/status.hpp listener.cpp listener.h message_connection_ops.cpp message_connection_ops.hpp server_connection.cpp server_connection.hpp client_connection.cpp client_connection.hpp)