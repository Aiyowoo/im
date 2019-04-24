
ADD_EXECUTABLE(reverse_echo_client main.cpp reverse_echo_client_connection.cpp reverse_echo_client_connection.hpp)
TARGET_LINK_LIBRARIES(reverse_echo_client avenue comm fmt::fmt fmt::fmt-header-only
                      ${Boost_SYSTEM_LIBRARY} ${Boost_DATE_TIME_LIBRARY} OpenSSL::SSL OpenSSL::Crypto)