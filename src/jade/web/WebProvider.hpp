#pragma once

#include <crow.h>

namespace jade {

class Server;

namespace WebProvider {
extern void init(Server* server);

extern void getRootIndex(Server* server, crow::request& req, crow::response& res);
extern void getLogin(Server* server, crow::request& req, crow::response& res);
extern void getSignup(Server* server, crow::request& req, crow::response& res);
extern void getLicenses(Server* server, crow::request& req, crow::response& res);

extern void getBooks(Server* server, crow::request& req, crow::response& res);

extern void getBookDetails(Server* server, crow::request& req, crow::response& res, int bookID);
extern void getBookReader(Server* server, crow::request& req, crow::response& res, int bookID);

extern void getAdminSettings(Server* server, crow::request& req, crow::response& res);
extern void getHealth(Server* server, crow::request& req, crow::response& res);

}

}
