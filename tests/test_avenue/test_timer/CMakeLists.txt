
ADD_EXECUTABLE(test_timer main.cpp logger.hpp)
TARGET_LINK_LIBRARIES(test_timer fmt::fmt fmt::fmt-header-only GTest::GTest GTest::Main avenue comm
${Boost_SYSTEM_LIBRARY} ${Boost_DATE_TIME_LIBRARY})