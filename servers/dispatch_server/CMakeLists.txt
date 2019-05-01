CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

PROJECT(dispatch_server CXX)

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

ADD_EXECUTABLE(dispatch_server main.cpp query_connection.cpp query_connection.hpp
stat_connection.cpp stat_connection.hpp server_stat_manager.cpp server_stat_manager.hpp
self_stat_connection.cpp self_stat_connection.hpp)
TARGET_LINK_LIBRARIES(dispatch_server avenue fmt::fmt fmt::fmt-header-only ${protobuf_LIBRARIES}
${Boost_DATE_TIME_LIBRARY} ${Boost_SYSTEM_LIBRARY} OpenSSL::SSL OpenSSL::Crypto comm)

INSTALL(TARGETS dispatch_server RUNTIME DESTINATION servers/bin)