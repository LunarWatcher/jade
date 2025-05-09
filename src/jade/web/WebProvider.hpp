#pragma once

#include <crow.h>

namespace jade {

class Server;

namespace WebProvider {
extern void init(Server* server);

extern void getRootIndex(Server* server, crow::request& req, crow::response& res);
extern void getLogin(Server* server, crow::request& req, crow::response& res);
extern void getSignup(Server* server, crow::request& req, crow::response& res);

extern void getAdminSettings(Server* server, crow::request& req, crow::response& res);

}

}
