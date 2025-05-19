#pragma once

#include "jade/data/db/Book.hpp"
#include <crow.h>

namespace jade {
class Server;
namespace BookAPI {

extern void init(Server* server);

extern void getImage(Server* server, crow::request& req, crow::response& res, int bookID);
extern void getBook(Server* server, crow::request& req, crow::response& res, int bookID);

extern void postEditBook(Server* server, crow::request& req, crow::response& res, int bookID);

}
}
