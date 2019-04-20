//
// Created by Administrator on 2018/12/28.
//

#include "status.hpp"

#ifdef WIN32

#include <Windows.h>
#else
#include <errno.h>
#include <string.h>
#endif

status::status(): code_(0) {}

status::status(status::status_code code):code_(code) {}

status::status(status::status_code code, const std::string &message) :
    code_(code), message_(message) {}

void status::clear() {
    code_ = 0;
    message_.clear();
}

void status::assign(status::status_code code, const std::string &message) {
    code_ = code;
    message_ = message;
}

status::operator bool() const {
    return code_ == 0;
}

status status::system_error() {
    status s;
#ifdef WIN32
    DWORD last_error = ::GetLastError();

#else
    int last_error = errno;
#endif
    if(last_error != 0) {
        s.assign(last_error, get_error_message(last_error));
    }
    return s;
}

std::string status::get_error_message(int error_no) {
    std::string message;
#ifdef WIN32
    LPVOID ptr = nullptr;
    DWORD ret = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_MAX_WIDTH_MASK, nullptr, error_no,
            0, (LPSTR)&ptr, 0, nullptr);
    if(ret && ptr != nullptr) {
        message = reinterpret_cast<char*>(ptr);
        LocalFree(ptr);
    }
#else
    char *ptr = strerror(error_no);
    if(ptr != nullptr) {
        message = ptr;
    }
#endif
    return message;
}

status::status_code status::code() const {
    return code_;
}

const std::string &status::message() const {
    return message_;
}


