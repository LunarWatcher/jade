#pragma once

#include <crow.h>

namespace jade {
class Server;
namespace BookAPI {

extern void init(Server* server);

extern void getImage(Server* server, crow::request& req, crow::response& res, int bookID);

}
}
