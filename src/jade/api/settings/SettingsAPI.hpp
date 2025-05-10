#pragma once

#include <crow.h>

namespace jade {
class Server;
namespace Settings {

extern void init(Server* server);

extern void createLibrary(Server* server, crow::request& req, crow::response& res);

}
}
