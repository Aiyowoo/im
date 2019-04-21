

ADD_EXECUTABLE(reverse_echo_server reverse_echo_server.cpp reverse_echo_server.hpp main.cpp logger.hpp)
TARGET_LINK_LIBRARIES(reverse_echo_server avenue comm fmt::fmt fmt::fmt-header-only
                      OpenSSL::SSL OpenSSL::Crypto ${Boost_SYSTEM_LIBRARY} ${Boost_DATE_TIME_LIBRARY})