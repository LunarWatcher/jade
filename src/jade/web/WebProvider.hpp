#pragma once

#include <crow.h>

namespace jade {

class Server;

namespace WebProvider {
extern void init(Server* server);

extern void getRootIndex(Server* server, crow::request& req, crow::response& res);

}

}
