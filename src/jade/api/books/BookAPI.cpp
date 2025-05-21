#include "BookAPI.hpp"
#include "jade/data/BookRequests.hpp"
#include "jade/server/AuthWall.hpp"
#include "jade/util/ResponseUtil.hpp"

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

    CROW_ROUTE(server->app, "/api/books/<int>/edit")
        .methods(crow::HTTPMethod::POST)
        .CROW_MIDDLEWARES(server->app, APINeedsAdmin)
        (std::bind(postEditBook, server, _1, _2, _3));
}

void BookAPI::getImage(Server* server, crow::request&, crow::response& res, int bookID) {
    auto theoreticalImgPath = server->getConfig().thumbnailCacheDir / (std::to_string(bookID) + ".jpg");
    spdlog::debug("Looking for cover for {} in {}", bookID, theoreticalImgPath.string());
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
void BookAPI::getBook(Server* server, crow::request&, crow::response& res, int bookID) {
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

void BookAPI::postEditBook(Server* server, crow::request& req, crow::response& res, int bookID) {
    auto book = server->lib->getBook(bookID);

    if (!book.has_value()) {
        res = JSONMessageResponse("Invalid book");
        res.code = 404;
        res.end();
        return;
    }
    BookRequest data = nlohmann::json::parse(req.body);
    spdlog::debug("Edit: parsed {}", req.body);

    server->pool->acquire<void>([&](pqxx::connection& conn) {
        pqxx::work w(conn);

        w.exec(R"(
        UPDATE Jade.Books
        SET
            Title = $2,
            ISBN = $3,
            Description = $4
        WHERE BookID = $1
        )", pqxx::params {
            bookID,
            data.title,
            data.isbn,
            data.description
        });
        spdlog::debug("Updated Jade.books");

        w.exec("DELETE FROM Jade.BookTags WHERE BookID = $1", pqxx::params { bookID });
        w.exec("DELETE FROM Jade.BookAuthors WHERE BookID = $1", pqxx::params { bookID });
        spdlog::debug("Dropped jade.booktags and bookauthors");

        for (auto& tag : data.tags) {
            spdlog::debug("Inserting tag: {}", tag);
            w.exec(R"(
            INSERT INTO Jade.Tags (TagName)
            VALUES ($1) ON CONFLICT (TagName) DO NOTHING
            )", pqxx::params {
                tag
            });
            w.exec(R"(
            INSERT INTO Jade.BookTags (BookID, TagID)
            SELECT $1, bt.TagID
            FROM Jade.Tags bt 
            WHERE bt.TagName = $2
            )", pqxx::params {
                bookID,
                tag
            });
        }
        for (auto& author : data.authors) {
            spdlog::debug("Inserting author: {}", author);
            w.exec(R"(
            INSERT INTO Jade.Authors (AuthorName)
            VALUES ($1) ON CONFLICT (AuthorName) DO NOTHING
            )", pqxx::params {
                author
            });
            w.exec(R"(
            INSERT INTO Jade.BookAuthors (BookID, AuthorID)
            SELECT $1, ba.AuthorID
            FROM Jade.Authors ba 
            WHERE ba.AuthorName = $2
            )", pqxx::params {
                bookID,
                author
            });
        }
        w.commit();
        spdlog::debug("Changes committed");
    });
    res = JSONMessageResponse("ok");
    res.code = 200;
    res.end();
}

}
