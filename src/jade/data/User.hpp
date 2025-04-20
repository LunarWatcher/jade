#pragma once

#include <string>

namespace jade {

struct User {
    std::string username;
    std::string password;
    int hashVersion;

    bool isAdmin;
    bool canUpload;
};

}
