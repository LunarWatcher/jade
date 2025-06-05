#pragma once

#include <string>

namespace jade {

struct User {
    long long userId;
    std::string username;
    bool isAdmin;
};

}
