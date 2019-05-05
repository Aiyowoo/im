CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

PROJECT(client_hub CXX)

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
LINK_DIRECTORIES($ENV{IM_INSTALL_PATH}/lib/servers)

ADD_EXECUTABLE(client_hub main.cpp user_connection.cpp user_connection.hpp
user_manager.cpp user_manager.hpp message_connection.cpp message_connection.hpp
message_connection_pool.cpp message_connection_pool.hpp config.cpp config.hpp)
TARGET_LINK_LIBRARIES(client_hub avenue fmt::fmt fmt::fmt-header-only ${protobuf_LIBRARIES}
${Boost_DATE_TIME_LIBRARY} ${Boost_SYSTEM_LIBRARY} OpenSSL::SSL OpenSSL::Crypto comm server_common)

INSTALL(TARGETS client_hub RUNTIME DESTINATION servers/bin)