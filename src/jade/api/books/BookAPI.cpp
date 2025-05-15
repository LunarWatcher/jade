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
    auto theoreticalImgPath = server->getConfig().thumbnailCacheDir / (std::to_string(bookID) + ".jpg");
    if (!std::filesystem::exists(theoreticalImgPath)) {
        res.code = 404;
        res.end();
        return;
    }

    res.set_static_file_info(theoreticalImgPath.string());
    res.set_header("Cache-Control", "public, max-age=2592000");
    res.set_header("Age", "0");
    res.end();
}

}
