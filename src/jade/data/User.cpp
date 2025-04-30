#include "User.hpp"

void jade::from_json(const nlohmann::json& src, User& dest) {
    src.at("userId").get_to(dest.userId);
    src.at("username").get_to(dest.username);
    src.at("isAdmin").get_to(dest.isAdmin);
}

void jade::to_json(nlohmann::json& dest, const User& src) {
    dest = {
        {"userId", src.userId},
        {"username", src.username},
        {"isAdmin", src.isAdmin}
    };
}
