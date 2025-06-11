#include "WebProvider.hpp"
#include "crow/common.h"
#include "jade/core/ContextProvider.hpp"
#include "jade/core/Typedefs.hpp"
#include "jade/data/BookRequests.hpp"
#include "jade/server/AuthWall.hpp"
#include "jade/server/SessionMiddleware.hpp"
#include "jade/util/Util.hpp"
#include <jade/core/Macros.hpp>
#include <jade/core/Server.hpp>

namespace jade {


void WebProvider::init(Server* server) {
    using namespace std::placeholders;
    CROW_ROUTE(server->app, "/")
        .methods(crow::HTTPMethod::GET)
        .CROW_MIDDLEWARES((server->app), NeedsAuthed)
        (JADE_CALLBACK_BINDING(getRootIndex));
    CROW_ROUTE(server->app, "/books.html")
        .methods(crow::HTTPMethod::GET)
        .CROW_MIDDLEWARES((server->app), NeedsAuthed)
        (JADE_CALLBACK_BINDING(getBooks));
    CROW_ROUTE(server->app, "/licenses.html")
        .methods(crow::HTTPMethod::GET)
        (JADE_CALLBACK_BINDING(getLicenses));
    CROW_ROUTE(server->app, "/books/<int>/details.html")
        .methods(crow::HTTPMethod::GET)
        .CROW_MIDDLEWARES((server->app), NeedsAuthed)
        (std::bind(getBookDetails, server, _1, _2, _3));
    CROW_ROUTE(server->app, "/books/<int>/reader.html")
        .methods(crow::HTTPMethod::GET)
        .CROW_MIDDLEWARES((server->app), NeedsAuthed)
        (std::bind(getBookReader, server, _1, _2, _3));

    CROW_ROUTE(server->app, "/settings/system.html")
        .methods(crow::HTTPMethod::GET)
        .CROW_MIDDLEWARES((server->app), NeedsAdmin)
        (JADE_CALLBACK_BINDING(getAdminSettings));
    CROW_ROUTE(server->app, "/settings/health.html")
        .methods(crow::HTTPMethod::GET)
        .CROW_MIDDLEWARES((server->app), NeedsAdmin)
        (JADE_CALLBACK_BINDING(getHealth));

    CROW_ROUTE(server->app, "/help/search.html")
        .methods(crow::HTTPMethod::GET)
        (JADE_CALLBACK_BINDING(getSearchHelp));

    CROW_ROUTE(server->app, "/auth/login.html")
        .methods(crow::HTTPMethod::GET)
        (JADE_CALLBACK_BINDING(getLogin));
    CROW_ROUTE(server->app, "/auth/signup.html")
        .methods(crow::HTTPMethod::GET)
        (JADE_CALLBACK_BINDING(getSignup));

    // Redirects {{{
    CROW_ROUTE(server->app, "/index.html")
        .methods(crow::HTTPMethod::GET)
        ([](crow::response& res) { res.redirect_perm("/"); });
    // }}}
    
    // Does not work due to https://github.com/CrowCpp/Crow/issues/859
    //CROW_CATCHALL_ROUTE(server->app)
        //([server](const crow::request& req, crow::response& res) {

            //static ContextProvider::PageContext ctx {
                //.pageTitle = "Error",
                //.pageDescription = "An error happened",
                //.pageFile = "error.mustache"
            //};

            //auto context = ContextProvider::buildBaseContext(
                //ContextProvider::ContextScope::USER, req, ctx, server
            //);
            //context["ErrorCode"] = res.code;
            //context["ErrorMessage"] = "404: page not found!";

            //res.set_header("Content-Type", ContentType::html);
            //res.body = ContextProvider::getBaseTemplate()
                //.render_string(context);
        //});

}

void WebProvider::getRootIndex(Server* server, crow::request& req, crow::response& res) {
    auto userCtx = server->app.get_context<MSessionMiddleware>(req);
    if (!userCtx.data || !userCtx.data->user) {
        res.redirect("/auth/login.html");
        res.end();
        return;
    }
    static ContextProvider::PageContext pageCtx {
        .pageTitle = "Ebooks | Jade",
        .pageDescription = "Jade - an eBook reader",
        .pageFile = "index.mustache",
        .includeSidebar = true
    };
    auto page = ContextProvider::getBaseTemplate();
    auto ctx = ContextProvider::buildBaseContext(
        ContextProvider::USER, req, pageCtx, server
    );

    res = page.render(ctx);
    res.end();
}

void WebProvider::getLogin(Server* server, crow::request& req, crow::response& res) {
    auto userCtx = server->app.get_context<MSessionMiddleware>(req);
    if (userCtx.data && userCtx.data->user) {
        res.redirect("/");
        res.end();
        return;
    }
    static ContextProvider::PageContext pageCtx {
        .pageTitle = "Log in | Jade",
        .pageDescription = "Log in to Jade",
        .pageFile = "auth/login.mustache",
        .pageScripts = {
            "/static/js/auth.js",
            "/static/js/pages/login.js",
        },
    };
    auto page = ContextProvider::getBaseTemplate();
    auto ctx = ContextProvider::buildBaseContext(0, req, pageCtx, server);

    res = page.render(ctx);
    res.end();
}

void WebProvider::getSignup(Server* server, crow::request& req, crow::response& res) {
    auto userCtx = server->app.get_context<MSessionMiddleware>(req);
    if (userCtx.data && userCtx.data->user) {
        res.redirect("/");
        res.end();
        return;
    }
    static ContextProvider::PageContext pageCtx {
        .pageTitle = "Sign up | Jade",
        .pageDescription = "Sign up to Jade",
        .pageFile = "auth/signup.mustache",
        .pageScripts = {
            "/static/js/auth.js",
            "/static/js/pages/signup.js",
        }
    };
    auto page = ContextProvider::getBaseTemplate();
    auto ctx = ContextProvider::buildBaseContext(0, req, pageCtx, server);

    res = page.render(ctx);
    res.end();
}

void WebProvider::getAdminSettings(Server* server, crow::request& req, crow::response& res) {
    auto userCtx = server->app.get_context<MSessionMiddleware>(req);
    static ContextProvider::PageContext pageCtx {
        .pageTitle = "Admin settings | Jade",
        .pageDescription = "Admin settings",
        .pageFile = "settings/admin.mustache",
        .pageScripts = {
            "/static/js/admin-settings.js"
        }
    };
    auto page = ContextProvider::getBaseTemplate();
    auto ctx = ContextProvider::buildBaseContext(
        ContextProvider::USER | ContextProvider::LIBRARIES,
        req, pageCtx, server
    );

    res = page.render(ctx);
    res.end();
}

void WebProvider::getHealth(Server* server, crow::request& req, crow::response& res) {
    auto userCtx = server->app.get_context<MSessionMiddleware>(req);
    static ContextProvider::PageContext pageCtx {
        .pageTitle = "Diagnostics | Jade",
        .pageDescription = "Jade system diagnostics",
        .pageFile = "settings/health.mustache"
    };
    auto page = ContextProvider::getBaseTemplate();
    auto ctx = ContextProvider::buildBaseContext(
        ContextProvider::USER | ContextProvider::LIBRARIES,
        req, pageCtx, server
    );

    ctx["Health"] = server->healthCore.getJSONResults();

    res = page.render(ctx);
    res.end();
}

void WebProvider::getBooks(Server* server, crow::request& req, crow::response& res) {
    auto userCtx = server->app.get_context<MSessionMiddleware>(req);
    static ContextProvider::PageContext pageCtx {
        .pageTitle = "Library | Jade",
        .pageDescription = "Ebook library",
        .pageFile = "listings/books.mustache",
        .pageScripts = {
            "/static/js/pages/library.js",
            "/static/js/components/paginator.js",
            "/static/js/components/filter.js"

        },
        .includeSidebar = true,
    };

    // Note: page IDs are assumed to be 0-indexed. The only place they should be 1-indexed is in the UI, and potentially
    // briefly in a UI-backend translation layer
    auto* pageIdStr = req.url_params.get("page");
    auto* pageSizeStr = req.url_params.get("pagesize");
    size_t pageId = 0;
    size_t pageSize = 50;

    if (pageIdStr != nullptr && strlen(pageIdStr) > 0) {
        pageId = std::stoull(pageIdStr);
        if (pageId != 0) {
            pageId -= 1;
        }
    }

    if (pageSizeStr != nullptr && strlen(pageSizeStr) > 0) {
        pageSize = std::stoull(pageSizeStr);
        if (pageSize > 200) {
            pageSize = 200;
        }
        if (pageSize == 0) {
            pageSize = 50;
        }
    }

    auto* searchStr = req.url_params.get("search");
    SearchRequest searchQuery;
    if (searchStr != nullptr) {
        try {
            searchQuery = SearchRequest::parse(searchStr);
        } catch (const std::exception& e) {
            spdlog::error("{}", e.what());

            // TODO: prevent invalid searches from hiding the searchbar and shit
            res.body = "Invalid search query";
            res.code = 400;
            res.end();
            return;
        }
    }

    auto page = ContextProvider::getBaseTemplate();
    auto ctx = ContextProvider::buildBaseContext(
        ContextProvider::USER,
        req, pageCtx, server
    );

    auto results = server->lib->getBooks(pageId, pageSize, searchQuery);

    if (pageId >= results.totalPages && pageId != 0) {

        res.body = "You provided an invalid page ID to your search. Did you incorrectly edit the parameter yourself?";
        res.code = 400;
        res.end();
        return;
    }

    ctx["Books"] = std::move(
        Util::vec2json(results.res)
    );
    ctx["Pages"] = {
        {"TotalSingles", results.totalResults},
        {"Count", results.totalPages},
        {"CurrentPage", pageId + 1}
    };

    res = page.render(ctx);
    res.end();
}

void WebProvider::getBookDetails(Server* server, crow::request& req, crow::response& res, int bookID) {
    auto userCtx = server->app.get_context<MSessionMiddleware>(req);
    auto book = server->lib->getBook(bookID);

    if (!book.has_value()) {
        res.body = "Book not found";
        res.code = 404;
        res.end();
        return;
    }

    ContextProvider::PageContext pageCtx {
        .pageTitle = book->title + " | Jade",
        .pageDescription = "Ebook library",
        .pageFile = "listings/book.mustache",
        .pageScripts = {
            "/static/js/pages/book.js"
        },
        .includeSidebar = true,
    };

    if (userCtx.data->user->isAdmin) {
        pageCtx.pageScripts.push_back(
            "/static/js/pages/book-admin.js"
        );
    }

    auto page = ContextProvider::getBaseTemplate();
    auto ctx = ContextProvider::buildBaseContext(
        ContextProvider::USER,
        req, pageCtx, server
    );

    ctx["Book"] = std::move(
        book->toJson()
    );

    res = page.render(ctx);
    res.end();

}

void WebProvider::getBookReader(Server* server, crow::request& req, crow::response& res, int bookID) {
    auto userCtx = server->app.get_context<MSessionMiddleware>(req);
    auto book = server->lib->getBook(bookID);

    if (!book.has_value()) {
        res.body = "Book not found";
        res.code = 404;
        res.end();
        return;
    }

    ContextProvider::PageContext pageCtx {
        .pageTitle = book->title + " | Jade Reader View",
        .pageDescription = "Ebook library",
        .pageFile = "reader.mustache",
        .pageModules = {
            "/static/js/reader.js"
        },
        .pageCSS = {"/static/css/foliate.css"},
        .hideFooter = true
    };

    auto page = ContextProvider::getBaseTemplate();
    auto ctx = ContextProvider::buildBaseContext(
        ContextProvider::USER,
        req, pageCtx, server
    );

    ctx["Book"] = std::move(
        book->toJson()
    );

    res = page.render(ctx);
    res.end();

}

void WebProvider::getLicenses(Server* server, crow::request& req, crow::response& res) {
    ContextProvider::PageContext pageCtx {
        .pageTitle = "Open-source licenses | Jade",
        .pageDescription = "Open-source licenses",
        .pageFile = "licenses.mustache",
    };

    auto page = ContextProvider::getBaseTemplate();
    auto ctx = ContextProvider::buildBaseContext(
        ContextProvider::USER,
        req, pageCtx, server
    );

    res = page.render(ctx);
    res.end();
}

void WebProvider::getSearchHelp(Server* server, crow::request& req, crow::response& res) {

    ContextProvider::PageContext pageCtx {
        .pageTitle = "Search help | Jade",
        .pageDescription = "Search help",
        .pageFile = "help/search.mustache",
    };

    auto page = ContextProvider::getBaseTemplate();
    auto ctx = ContextProvider::buildBaseContext(
        ContextProvider::USER,
        req, pageCtx, server
    );

    res = page.render(ctx);
    res.end();
}

}
