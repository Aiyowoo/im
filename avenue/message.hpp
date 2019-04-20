#pragma once
#ifndef AVENUE_MESSAGE_H
#define AVENUE_MESSAGE_H

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#endif

#include <comm/status.hpp>

#include <fmt/format.h>

#include <boost/noncopyable.hpp>

#include <cstdint>
#include <memory>
#include <string>

namespace avenue {

struct message_header {
    uint32_t service_id;
    uint32_t message_id;
    uint8_t is_request;
    uint8_t identify[3];
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

    void validate(status &s) {
        s.clear();
        if (identify[0] == 0xFF && identify[1] == 0x01 && identify[2] == 0xA3) {
            return;
        }
        s.assign(status::RUNTIME_ERROR, "not a message header?");
    }
};

class message : boost::noncopyable {
    message_header header_;
    char *data_;

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

    const message_header *get_header() const;

    message_header *get_header();

    void get_body(const char *&body, uint32_t &len);

    void get_body(char *&body, uint32_t &len);

    void prepare(uint32_t body_len);

    void validate(status &s);

    void swap(message &other) noexcept;

private:
    void clear();

    void set_body_len(uint32_t body_len);

    uint32_t change_bytes_order(uint32_t value) const;
};

void swap(message &lhs, message &rhs) noexcept;

}

template<>
struct fmt::formatter<avenue::message> {
    template<typename ParseContext>
    constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

    template<typename FormatContext>
    auto format(const avenue::message &msg, FormatContext &ctx) {
        return format_to(ctx.out, "%s message service_id[%d] message_id[%d] sequence[%d] body_len[%d]",
                         (msg.is_request() ? "request" : "response"), msg.get_service_id(),
                         msg.get_message_id(), msg.get_sequence(), msg.get_body_len());
    }
};

#endif
