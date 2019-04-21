
ADD_EXECUTABLE(reverse_echo_client main.cpp)
TARGET_LINK_LIBRARIES(reverse_echo_client avenue comm OpenSSL::SSL OpenSSL::Crypto fmt::fmt fmt::fmt-header-only
                      ${Boost_SYSTEM_LIBRARY} ${Boost_DATE_TIME_LIBRARY})