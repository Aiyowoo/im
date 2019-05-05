CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

PROJECT(server_common CXX)

FIND_PACKAGE(Boost REQUIRED COMPONENTS system date_time)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})

FIND_PACKAGE(fmt CONFIG REQUIRED)

FIND_PACKAGE(protobuf REQUIRED)
INCLUDE_DIRECTORIES(${Protobuf_INCLUDE_DIRS})

# OpenSSL
FIND_PACKAGE(OpenSSL REQUIRED)

INCLUDE_DIRECTORIES($ENV{IM_INSTALL_PATH}/include)
LINK_DIRECTORIES($ENV{IM_INSTALL_PATH}/lib/avenue)
LINK_DIRECTORIES($ENV{IM_INSTALL_PATH}/lib/comm)

ADD_LIBRARY(server_common config.cpp config.hpp logger.hpp constants.hpp error.cpp error.hpp)

TARGET_LINK_LIBRARIES(server_common fmt::fmt fmt::fmt-header-only ${protobuf_LIBRARIES}
${Boost_DATE_TIME_LIBRARY} ${Boost_SYSTEM_LIBRARY} OpenSSL::SSL OpenSSL::Crypto comm)

INSTALL(TARGETS server_common DESTINATION lib/servers)
INSTALL(FILES config.hpp logger.hpp constants.hpp error.hpp DESTINATION include/servers)