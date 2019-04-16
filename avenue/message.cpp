#include "message.hpp"

#include <cassert>

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"

#ifdef WIN32

#include <WinSock2.h>

#else

#include <apra/inet.h>

#endif

namespace avenue {

message::message() : message(0, 0, 0) {
}

message::message(uint32_t service_id, uint32_t message_id, uint32_t sequence)
        : header_(service_id, message_id, sequence, 0, 0), data_(nullptr) {
}

message::message(message &&other) noexcept: data_(nullptr) {
    if (this == &other) {
        return;
    }

    using std::swap;
    swap(this->header_, other.header_);
    swap(this->data_, other.data_);
    other.clear();
}

message &message::operator=(message &&other) noexcept {
    if (this == &other) {
        return *this;
    }

    using std::swap;
    swap(this->header_, other.header_);
    swap(this->data_, other.data_);
    other.clear();
    return *this;
}

message::~message() {
    clear();
}

uint32_t message::get_service_id() const {
    return header_.service_id;
}

void message::set_service_id(uint32_t service_id) {
    header_.service_id = service_id;
}

uint32_t message::get_message_id() const {
    return header_.message_id;
}

void message::set_message_id(uint32_t message_id) {
    header_.message_id = message_id;
}

bool message::is_request() const {
    return header_.is_request;
}

void message::set_is_request(bool value) {
    header_.is_request = value;
}

uint32_t message::get_sequence() const {
    return header_.sequence;
}

void message::set_sequence(uint32_t sequence) {
    header_.sequence = sequence;
}

uint32_t message::get_options() const {
    return header_.options;
}

void message::set_options(uint32_t options) {
    header_.options = options;
}

uint32_t message::get_body_len() const {
    return header_.body_len;
}

void message::get_body(const char *&ptr, uint32_t &len) const {
    ptr = data_;
    len = header_.body_len;
}

void message::set_body(const char *data, uint32_t len) {
    auto *ptr = new char[len];
    std::copy_n(data, len, ptr);
    delete[] data_;
    data_ = ptr;
    header_.body_len = len;
}

void message::set_body(std::unique_ptr<char[]> data_ptr, uint32_t len) {
    assert(data_ptr.get() != data_);
    delete[] data_;
    data_ = data_ptr.release();
    header_.body_len = len;
}

void handle_bytes_order(message_header &header) {
    header.service_id = htonl(header.service_id);
}

std::string message::serialize() const {
    std::string buffer(sizeof(header_) + header_.body_len, 0);
    auto ptr = const_cast<char *>(buffer.c_str());
    handle_bytes_order(*reinterpret_cast<message_header *>(ptr));
    std::copy_n(data_, header_.body_len, ptr + sizeof(header_));
    return buffer;
}

void message::clear() {
    if (data_) {
        delete[] data_;
        data_ = nullptr;
        header_.body_len = 0;
    }
}

}

#pragma clang diagnostic pop