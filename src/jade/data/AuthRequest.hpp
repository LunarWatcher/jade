#pragma once

#include <string>
#include <nlohmann/json.hpp>

namespace jade {

struct LoginRequest {
    std::string username;
    std::string password;
};

// Left for future compatibility if I add more fields
struct SignupRequest : public LoginRequest {

};

void from_json(const nlohmann::json& j, LoginRequest& o);
void from_json(const nlohmann::json& j, SignupRequest& o);

}
