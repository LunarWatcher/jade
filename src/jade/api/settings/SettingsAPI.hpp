#pragma once

#include <crow.h>

namespace jade {
class Server;
namespace Settings {

extern void init(Server* server);

extern void postCreateLibrary(Server* server, crow::request& req, crow::response& res);

extern void postRefreshLibraries(Server* server, crow::request& req, crow::response& res);

}
}
