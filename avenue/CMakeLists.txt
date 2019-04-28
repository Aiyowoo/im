
CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

PROJECT(avenue)

FIND_PACKAGE(fmt CONFIG REQUIRED)

FIND_PACKAGE(Boost REQUIRED)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})

INCLUDE_DIRECTORIES($ENV{IM_INSTALL_PATH}/include)
LINK_DIRECTORIES($ENV{IM_INSTALL_PATH}/lib/comm)

IF(WIN32)

ADD_DEFINITIONS(/DDEBUG)

ENDIF(WIN32)

ADD_LIBRARY(avenue message.cpp message.hpp tcp_server.cpp tcp_server.hpp
            listener.cpp listener.hpp
            logger.cpp logger.hpp details/default_logger.hpp details/log_helper.hpp message_connection.cpp message_connection.hpp 
			server_connection.cpp server_connection.hpp client_connection.cpp client_connection.hpp timer.cpp timer.hpp)
TARGET_LINK_LIBRARIES(avenue comm fmt::fmt fmt::fmt-header-only)

INSTALL(TARGETS avenue DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/avenue)
INSTALL(FILES tcp_server.hpp server_connection.hpp message.hpp listener.hpp client_connection.hpp logger.hpp message_connection.hpp timer.hpp
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include/avenue)