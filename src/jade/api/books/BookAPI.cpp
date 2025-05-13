#include "BookAPI.hpp"
#include "jade/server/AuthWall.hpp"

#include <jade/core/Server.hpp>

namespace jade {

void BookAPI::init(Server *server) {
    using namespace std::placeholders;
    CROW_ROUTE(server->app, "/api/books/<int>/cover")
        .methods(crow::HTTPMethod::GET)
        .CROW_MIDDLEWARES(server->app, APINeedsAuthed)
        (std::bind(getImage, server, _1, _2, _3));
}

void BookAPI::getImage(Server* server, crow::request& req, crow::response& res, int bookID) {
    spdlog::info("Image requested for {}", bookID);
}

}
