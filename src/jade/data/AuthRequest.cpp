#include "AuthRequest.hpp"

namespace jade {

void from_json(const nlohmann::json& j, LoginRequest& o) {
    j.at("username").get_to(o.username);
    j.at("password").get_to(o.password);
}

void from_json(const nlohmann::json& j, SignupRequest& o) {
    j.at("username").get_to(o.username);
    j.at("password").get_to(o.password);
}

}
