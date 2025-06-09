#pragma once

#include "jade/data/User.hpp"
#include <crow.h>

namespace jade {

class Server;

namespace AuthAPI {

struct UserResponse {
    User user;
};

extern void init(Server* server);

extern void postLogin(Server* server, crow::request& req, crow::response& res);
extern void postSignup(Server* server, crow::request& req, crow::response& res);
extern void getLogout(Server* server, crow::request& req, crow::response& res);

}

}
