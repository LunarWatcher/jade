#pragma once

#include <string>

namespace jade {

struct LoginRequest {
    std::string username;
    std::string password;
};

struct SignupRequest {
    std::string username;
    std::string password;
};

}
