
FIND_PACKAGE(Boost REQUIRED COMPONENTS system date_time)
INCLUDE_DIRECTORIES(${BOOST_INCLUDE_DIRS})

FIND_PACKAGE(fmt CONFIG REQUIRED)

FIND_PACKAGE(OpenSSL REQUIRED)

FIND_PACKAGE(GTest REQUIRED)
INCLUDE_DIRECTORIES(${GTEST_INCLUDE_DIRS})

INCLUDE_DIRECTORIES($ENV{IM_INSTALL_PATH}/include)
LINK_DIRECTORIES($ENV{IM_INSTALL_PATH}/lib/avenue)
LINK_DIRECTORIES($ENV{IM_INSTALL_PATH}/lib/comm)
LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})
LINK_DIRECTORIES(${GTEST_LIBRARY_DIRS})

ADD_SUBDIRECTORY(reverse_echo_server)
ADD_SUBDIRECTORY(reverse_echo_client)
ADD_SUBDIRECTORY(test_timer)
