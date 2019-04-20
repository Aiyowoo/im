//
// Created by Administrator on 2018/12/28.
//

#ifndef GOPSAGENT_STATUS_HPP
#define GOPSAGENT_STATUS_HPP

#include <string>

class status {
public:
    enum {
        FILE_NOT_FOUND = -999,
        MD5_NOT_MATCH,
        PARSE_JSON_ERROR,
        SYSTEM_ERROR,
        UNKNOWN_ERROR,
        INVALID_HANDLE,
        PARAMETERS_ERROR,
        CMD_FAILED,
        TIMEOUT,
        RUNTIME_ERROR,
        TASK_ALREADY_RUNNING,
        TEMPORARY_UNAVAILABLE,
        NO_RESOURCES,
        DIST_FILE_DOWNLOAD_FAILED,
        OPERATION_CANCELLED,
        OK = 0
    };

    using status_code = int;

    status();

    status(status_code code);

    status(status_code code, const std::string &message);

    void clear();

    void assign(status_code code, const std::string &message);

    operator bool() const;

    status_code code() const;

    const std::string &message() const;

    static status system_error();

private:
    status_code code_;
    std::string message_;

    static std::string get_error_message(int error_no);
};


#endif //GOPSAGENT_STATUS_HPP
