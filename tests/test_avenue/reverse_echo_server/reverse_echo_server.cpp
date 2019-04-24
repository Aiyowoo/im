//  reverse_echo_server.cpp
// Created by m8792 on 2019/4/21.
// 2019/4/21 1:15

#include "reverse_echo_server.hpp"
#include "reverse_echo_connection.hpp"
#include "logger.hpp"

#include <cstring>

const char *PRIVATE_KEY = "-----BEGIN RSA PRIVATE KEY-----\n"
                          "MIIEpAIBAAKCAQEAtn3Be3y+2gmjwLg83P/pQCYKu4c9QZhJ5i60VE7DvkXv6zhp\n"
                          "X0wcRES68vjLyz4CkptqPGnIgJIGyQ0Infoe6BKWz6oeYDSIILunxemA8i5uLoXK\n"
                          "uG/tReFs7Dyp/LcgG8wFAWIWgsglszyjUAaMUu0cMe6cjfQW/ogTYId3NnYCJ0ox\n"
                          "rnTDAycI1fU5QpuA0WhWtmGrd6WUXHxDQLMbfIF9ahjofpSqwTy/EpSbOpxMAKeu\n"
                          "UMqpPi+A1D6I6WbL7L85bSoM6f2/UexJ1Ft6F7QSP6hwpZqCgGTGdjbKXCF/vOMJ\n"
                          "tY8F09+CP1br9akPu+XrUZWpL0CyoeRGqhAJ+wIDAQABAoIBAQCz1bOAhIrzBQLZ\n"
                          "HPXld08dhx+Wc4xbAr74VdghKMlezT7D1wCIB9HnOK/eVm1GKSKR5RPI2xnaEY0c\n"
                          "PXrbaytf2UnC9Usf1US8tRB9SUFn53du4yvFm48ACUi0eIevYasDmYbpOQcj9Caq\n"
                          "vbvWVc2cvsrFLfYbMYojZGdr/bLt96ygREb2cjTr6lbPXD89+nd9e6G3OcEFClKH\n"
                          "ulxh99hjF7FFPhxBL4+efu6QiuZo6fpV4jeJtlEPZ3spHV5DsxqpJzB2vOIN1Nsg\n"
                          "PHHiW9qh+NUT4Czp7a4Jnlc3+N2wGMzkyXXeVeBmZhn5pGANMWGCPa754rS2wGhi\n"
                          "fIFgOwOhAoGBAN/ClBSVObt/+2/tlaS8IME6uo4w4o+vchz+MjDTnQn4T593sNjn\n"
                          "Ri/wnCvexwBcuhmvwGb4fW5cKQOiYFE1YqVO1s2epy0VKqKw/eI3af4KheVCY/yr\n"
                          "DH0fLFtIQSMwHPnlYuVOnLRkHXTopU6EN85o9FmOEqyz9srcDMy80lmrAoGBANDI\n"
                          "+HDIB5Oaj2i/Y/U4Nq67/IuGkqVENsgj7OmDdBs7bORQK7uDAx10BERCc6wgUiHD\n"
                          "bAoljXQxp2CPreqDmYtl0smn8/0gKajb3jxM5sc0Hqs2cg3yn+f+h6ZKs5B8mwko\n"
                          "w0zAW2rzdtkBILtmZLDZfkJJEoEmtvGMVoUAvODxAoGBAIsHSxq+3XAyxPd2l1QT\n"
                          "0AY/h0px9VNxn2WES4sq8JgRQOfEZo1epa2DwoU/lkYTV04+dT2Fh+naOWGYsl/C\n"
                          "29f6dFyuBxFr5WHQtu6nn6PIxgj8h0Omw8u6YsUUz+QdMOPUMu5nOdabnfHV22A9\n"
                          "pq8JZ0l9jPzhlKXWs91CNTChAoGANLLlcqIqt2kF1aypcFX2B3jPr/ARx8FGCpJ2\n"
                          "1CIoBk+jlqVdxIC3IpIMGaakR8gIsxj+hjkgUTnPQ6JAhbcd5H+ZCulnvnaFzWdh\n"
                          "2tqWTaWsMA8YDsk073vhhWnJlSi+gTtWwGK1MHOvheqw9lxMAg+DvPtwvkSlXKbN\n"
                          "l9it1CECgYBOpeRLpatZC3QQ39vK6ks9y07zkDA1GKSg24vzzDe2DAvyEVnJ5Z9D\n"
                          "MjDR6L4QzOaPds1lOllyi2rikGSF0KX86GtU9c/eSJJ+R3lZ1itPDujLXwK/JuYC\n"
                          "PK8hLGksvolIrb/gO779h7Ssojk72RGymmzOI/thnd2O0dZKb7MUmw==\n"
                          "-----END RSA PRIVATE KEY-----";

const char *PUBLIC_KEY = "-----BEGIN CERTIFICATE-----\n"
                         "MIIDijCCAnKgAwIBAgIJAJS7/c6oGAiUMA0GCSqGSIb3DQEBCwUAMFoxCzAJBgNV\n"
                         "BAYTAkNOMREwDwYDVQQIDAhTaGFuZ0hhaTERMA8GA1UEBwwIU2hhbmdIYWkxCzAJ\n"
                         "BgNVBAoMAmltMQswCQYDVQQLDAJpbTELMAkGA1UEAwwCaW0wHhcNMTkwNDIxMDY1\n"
                         "NjA3WhcNMjAwNDIwMDY1NjA3WjBaMQswCQYDVQQGEwJDTjERMA8GA1UECAwIU2hh\n"
                         "bmdIYWkxETAPBgNVBAcMCFNoYW5nSGFpMQswCQYDVQQKDAJpbTELMAkGA1UECwwC\n"
                         "aW0xCzAJBgNVBAMMAmltMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA\n"
                         "tn3Be3y+2gmjwLg83P/pQCYKu4c9QZhJ5i60VE7DvkXv6zhpX0wcRES68vjLyz4C\n"
                         "kptqPGnIgJIGyQ0Infoe6BKWz6oeYDSIILunxemA8i5uLoXKuG/tReFs7Dyp/Lcg\n"
                         "G8wFAWIWgsglszyjUAaMUu0cMe6cjfQW/ogTYId3NnYCJ0oxrnTDAycI1fU5QpuA\n"
                         "0WhWtmGrd6WUXHxDQLMbfIF9ahjofpSqwTy/EpSbOpxMAKeuUMqpPi+A1D6I6WbL\n"
                         "7L85bSoM6f2/UexJ1Ft6F7QSP6hwpZqCgGTGdjbKXCF/vOMJtY8F09+CP1br9akP\n"
                         "u+XrUZWpL0CyoeRGqhAJ+wIDAQABo1MwUTAdBgNVHQ4EFgQUoqe9A/rhrckkFORu\n"
                         "/5rjFykQSHAwHwYDVR0jBBgwFoAUoqe9A/rhrckkFORu/5rjFykQSHAwDwYDVR0T\n"
                         "AQH/BAUwAwEB/zANBgkqhkiG9w0BAQsFAAOCAQEAizta8F5fYqhkU3ftf1e1o8n3\n"
                         "tGt6mBc7TJ8wHkz7gBGTGq4Oh/tY2kYW8yXc2Uv9wXJZGSO5Td5tqlg8C7r35Z8r\n"
                         "Vjzp1CmdWX/QJPUBvdvvbWibo9GAACFgKlYoV6rRJNnFerG8DdjhRsH+ExCfHF0h\n"
                         "iHkE1k7k6+zPm+/inmOXDN8Hr5j3ljdBicDr7ehjNxvr8pWgzvPczYyL7EaESTjE\n"
                         "lpJ4pkaycBvmVIJ7vg4cEtb8yd5LNJpsL4k74Ri25iDPrW9jMFDRIKFadQQErdbj\n"
                         "OtOIC/yFBY821qGzIa+JvbwsJudr5iMXXX2Hk0SnzFuhtvSuoojswefj3c5cWw==\n"
                         "-----END CERTIFICATE-----";

reverse_echo_server::reverse_echo_server() : ssl_context_(boost::asio::ssl::context::sslv23) {
    ssl_context_.use_certificate(boost::asio::const_buffer(PUBLIC_KEY, std::strlen(PUBLIC_KEY)),
                                 boost::asio::ssl::context::pem);
    ssl_context_.use_private_key(boost::asio::const_buffer(PRIVATE_KEY, std::strlen(PRIVATE_KEY)),
                                 boost::asio::ssl::context::pem);
}


void reverse_echo_server::run() {
    server_.listen("0.0.0.0", 54321, [this](boost::asio::ip::tcp::socket &socket) {
        on_receive_connection(socket);
    });
    server_.start(1);
}

void reverse_echo_server::on_receive_connection(boost::asio::ip::tcp::socket &socket) {
    auto conn_ptr = std::make_shared<reverse_echo_connection>(socket, ssl_context_);
    conn_ptr->run();
}
