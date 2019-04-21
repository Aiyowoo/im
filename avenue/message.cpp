
#include "message.hpp"

#ifdef WIN32

#include <WinSock2.h>

#else

#include <apra/inet.h>

#endif

#include <cassert>

namespace avenue {

message::message() : message(static_cast<uint32_t>(-1), static_cast<uint32_t>(-1), 0) {}

message::message(uint32_t service_id, uint32_t message_id, uint32_t sequence) {
    set_service_id(service_id);
    set_message_id(message_id);
    set_is_request(false);
    set_sequence(sequence);
    set_options(0);
    set_body_len(0);
    data_ = nullptr;
}

message::message(message &&other) noexcept {
    *this = std::move(other);
}

message &message::operator=(message &&other) noexcept {
    using std::swap;
    message tmp;
    swap(tmp, other);
    swap(*this, tmp);
    return *this;
}

message::~message() {
    clear();
}

uint32_t message::get_service_id() const {
    return change_bytes_order(header_.service_id);
}

void message::set_service_id(uint32_t service_id) {
    header_.service_id = change_bytes_order(service_id);
}


uint32_t message::get_message_id() const {
    return change_bytes_order(header_.message_id);
}

void message::set_message_id(uint32_t message_id) {
    header_.message_id = change_bytes_order(message_id);
}

bool message::is_request() const {
    return change_bytes_order(header_.is_request) != 0;
}

void message::set_is_request(bool value) {
    header_.is_request = static_cast<uint8_t>(value);
}

uint32_t message::get_sequence() const {
    return change_bytes_order(header_.sequence);
}

void message::set_sequence(uint32_t sequence) {
    header_.sequence = change_bytes_order(sequence);
}

uint32_t message::get_options() const {
    return change_bytes_order(header_.options);
}

void message::set_options(uint32_t options) {
    header_.options = change_bytes_order(options);
}

uint32_t message::get_body_len() const {
    return change_bytes_order(header_.body_len);
}

const message_header *message::get_header() const {
    return &header_;
}

message_header *message::get_header() {
    return &header_;
}

void message::get_body(const char *&body, uint32_t &len) {
    body = data_;
    len = get_body_len();
}

void message::get_body(char *&body, uint32_t &len) {
    body = data_;
    len = get_body_len();
}

void message::prepare(uint32_t body_len) {
    clear();
    data_ = new char[body_len];
    set_body_len(body_len);
}

void message::clear() {
    if (data_) {
        delete[] data_;
        data_ = nullptr;
    }
    set_body_len(0);
}

void message::set_body_len(uint32_t body_len) {
    header_.body_len = change_bytes_order(body_len);
}

uint32_t message::change_bytes_order(uint32_t value) const {
    return htonl(value);
}

void message::validate(status &s) {
    return header_.validate(s);
}

void message::swap(message &other) noexcept {
    using std::swap;
    swap(this->header_, other.header_);
    swap(data_, other.data_);
}

void swap(message &lhs, message &rhs) noexcept {
    lhs.swap(rhs);
}
}
