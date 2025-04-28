#pragma once

#include <crow.h>

namespace jade {

class Server;

namespace AuthAPI {

extern void init(Server* server);

extern void login(Server* server, crow::request& req, crow::response& res);
extern void signup(Server* server, crow::request& req, crow::response& res);

}

}
