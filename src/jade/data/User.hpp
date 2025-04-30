#pragma once

#include "nlohmann/json.hpp"
#include <string>

namespace jade {

struct User {
    long long userId;
    std::string username;
    bool isAdmin;
};

void from_json(const nlohmann::json& src, User& dest);
void to_json(nlohmann::json& src, const User& dest);

}
