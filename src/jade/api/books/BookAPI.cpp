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
    CROW_ROUTE(server->app, "/api/books/<int>/download")
        .methods(crow::HTTPMethod::GET)
        .CROW_MIDDLEWARES(server->app, APINeedsAuthed)
        (std::bind(getBook, server, _1, _2, _3));
}

void BookAPI::getImage(Server* server, crow::request& req, crow::response& res, int bookID) {
    auto theoreticalImgPath = server->getConfig().thumbnailCacheDir / (std::to_string(bookID) + ".jpg");
    if (!std::filesystem::exists(theoreticalImgPath)) {
        res.code = 404;
        res.end();
        return;
    }

    res.set_static_file_info(theoreticalImgPath.string());
    res.set_header(
        "Content-Disposition", 
        "attachment; filename=\"" + std::to_string(bookID) + ".jpg\""
    );
    res.set_header("Cache-Control", "public, max-age=2592000");
    res.set_header("Age", "0");
    res.end();
}
void BookAPI::getBook(Server* server, crow::request& req, crow::response& res, int bookID) {
    auto book = server->lib->getBook(bookID);
    if (!book.has_value()) {
        res.body = "Book not found";
        res.code = 404;
        res.end();
        return;
    }

    res.set_static_file_info_unsafe(book->fullPath.string());
    // Crow has extremely limited filetype support, and doesn't support several of the ebook types
    res.set_header(
        "Content-Type", 
        server->lib->MIMETYPES.at(
            *server->lib->validateExtension(book->fullPath)
        )
    );
    // TODO: C++23 should support `{:?}` to avoid stringstream, not sure if it's supported enough to migrate yet
    std::stringstream ss;
    ss << "attachment; filename="
        << std::quoted(book->fullPath.filename().string());
    res.add_header(
        "Content-Disposition", 
        ss.str()
    );
    res.add_header("Cache-Control", "public, max-age=2592000");
    res.add_header("Age", "0");
    res.end();
}

}
