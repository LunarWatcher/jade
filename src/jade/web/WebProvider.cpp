#include "WebProvider.hpp"
#include "jade/core/ContextProvider.hpp"
#include <jade/core/Macros.hpp>
#include <jade/core/Server.hpp>

namespace jade {


void WebProvider::init(Server* server) {
    CROW_ROUTE(server->app, "/")
        .methods(crow::HTTPMethod::GET)
        (JADE_CALLBACK_BINDING(getRootIndex));
    CROW_ROUTE(server->app, "/auth/login.html")
        .methods(crow::HTTPMethod::GET)
        (JADE_CALLBACK_BINDING(getLogin));

    CROW_ROUTE(server->app, "/index.html")
        .methods(crow::HTTPMethod::GET)
        ([](crow::response& res) { res.redirect("/"); });

}

void WebProvider::getRootIndex(Server* server, crow::request& req, crow::response& res) {
    static ContextProvider::PageContext pageCtx {
        .pageTitle = "Jade",
        .pageDescription = "Jade - an eBook reader",
        .pageFile = "index.mustache"
    };
    auto page = ContextProvider::getBaseTemplate();
    auto ctx = ContextProvider::buildBaseContext(0, req, pageCtx, server);

    res.body = page.render_string(ctx);
    res.set_header("Content-Type", "text/html");
    res.end();
}

void WebProvider::getLogin(Server* server, crow::request& req, crow::response& res) {
    static ContextProvider::PageContext pageCtx {
        .pageTitle = "Log in | Jade",
        .pageDescription = "Log in to Jade",
        .pageFile = "login.mustache"
    };
    auto page = ContextProvider::getBaseTemplate();
    auto ctx = ContextProvider::buildBaseContext(0, req, pageCtx, server);

    res.body = page.render_string(ctx);
    res.set_header("Content-Type", "text/html");
    res.end();
}

}
