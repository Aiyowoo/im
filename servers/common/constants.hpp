#ifndef SERVERS_COMMON_CONSTANTS_HPP
#define SERVERS_COMMON_CONSTANTS_HPP

#include <cstdint>
#include <chrono>

using user_id_type = uint64_t;

constexpr user_id_type INVALID_USER_ID = 0;

using device_type = uint32_t;

const device_type INVALID_DEVICE = 0;

constexpr std::chrono::system_clock::duration USER_REQUEST_TIMEOUT = std::chrono::seconds(1);

#endif // SERVERS_COMMON_CONSTANTS_HPP
