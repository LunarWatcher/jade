#pragma once

#include "jade/data/User.hpp"
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

struct LoginResponse {
    User user;
};

}
