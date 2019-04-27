
ADD_DEFINITIONS(/DDEBUG)

ADD_EXECUTABLE(reverse_echo_client main.cpp reverse_echo_client_connection.cpp reverse_echo_client_connection.hpp 
	core_dumper.cpp core_dumper.hpp multi_request_connection.cpp multi_request_connection.hpp)
TARGET_LINK_LIBRARIES(reverse_echo_client avenue comm fmt::fmt fmt::fmt-header-only
                      ${Boost_SYSTEM_LIBRARY} ${Boost_DATE_TIME_LIBRARY} OpenSSL::SSL OpenSSL::Crypto GTest::GTest GTest::Main)