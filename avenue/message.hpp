#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
#pragma once
#ifndef AVENUE_MESSAGE_H
#define AVENUE_MESSAGE_H

#include <boost/noncopyable.hpp>

#include <cstdint>
#include <memory>
#include <string>

namespace avenue {

struct message_header {
    uint32_t service_id;
    uint32_t message_id;
    uint8_t is_request;
    uint8_t  identify[3];
    uint32_t sequence;
    uint32_t options;
    uint32_t body_len;

    message_header() : service_id(-1), message_id(-1), is_request(0),
                       sequence(0), options(0), body_len(0) {
        identify[0] = 0xFF;
        identify[1] = 0x01;
        identify[2] = 0xA3;
    }

    message_header(uint32_t service_id, uint32_t message_id, uint32_t sequence,
                   uint32_t options, uint32_t body_len)
            : service_id(service_id), message_id(message_id), is_request(0),
              sequence(sequence), options(options), body_len(body_len) {
        identify[0] = 0xFF;
        identify[1] = 0x01;
        identify[2] = 0xA3;
    }
};

class message : boost::noncopyable {
    message_header header_;
    const char *data_;

public:
    message();

    message(uint32_t service_id, uint32_t message_id, uint32_t sequence);

    message(const message &&) = delete;

    message &operator=(const message &) = delete;

    message(message &&other) noexcept;

    message &operator=(message &&other) noexcept;

    ~message();

    uint32_t get_service_id() const;

    void set_service_id(uint32_t service_id);

    uint32_t get_message_id() const;

    void set_message_id(uint32_t message_id);

    bool is_request() const;

    void set_is_request(bool value);

    uint32_t get_sequence() const;

    void set_sequence(uint32_t sequence);

    uint32_t get_options() const;

    void set_options(uint32_t options);

    uint32_t get_body_len() const;

    void get_body(const char *&ptr, uint32_t &len) const;

    void set_body(const char *data, uint32_t len);

    void set_body(std::unique_ptr<char[]> data_ptr, uint32_t len);

    std::string serialize() const;

private:
    void clear();
};

}

#endif

#pragma clang diagnostic pop