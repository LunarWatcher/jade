#pragma once

#include "jade/db/security/Hash.hpp"
#include "spdlog/spdlog.h"
#include <openssl/rand.h>
#include <string>

namespace jade {

template <int Bytes = 64>
std::string generateToken() {
    unsigned char buf[Bytes];
    if (auto code = RAND_bytes(buf, Bytes); code != 1) {
        spdlog::error("OpenSSL failure, code = {}", code);
        throw std::runtime_error("OpenSSL failure");
    }

    return Hash::_detail::byteDecode<Bytes>(buf);
}

}
