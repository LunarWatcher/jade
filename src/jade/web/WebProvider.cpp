#include "WebProvider.hpp"
#include "crow/common.h"
#include "jade/core/ContextProvider.hpp"
#include "jade/core/Typedefs.hpp"
#include "jade/server/AuthWall.hpp"
#include "jade/server/SessionMiddleware.hpp"
#include <jade/core/Macros.hpp>
#include <jade/core/Server.hpp>

namespace jade {


void WebProvider::init(Server* server) {
    CROW_ROUTE(server->app, "/")
        .methods(crow::HTTPMethod::GET)
        .CROW_MIDDLEWARES((server->app), NeedsAuthed)
        (JADE_CALLBACK_BINDING(getRootIndex));

    CROW_ROUTE(server->app, "/settings/system.html")
        .methods(crow::HTTPMethod::GET)
        .CROW_MIDDLEWARES((server->app), NeedsAdmin)
        (JADE_CALLBACK_BINDING(getAdminSettings));

    CROW_ROUTE(server->app, "/auth/login.html")
        .methods(crow::HTTPMethod::GET)
        (JADE_CALLBACK_BINDING(getLogin));

    // Redirects {{{
    CROW_ROUTE(server->app, "/index.html")
        .methods(crow::HTTPMethod::GET)
        ([](crow::response& res) { res.redirect_perm("/"); });
    // }}}

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
        .pageFile = "index.mustache"
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
        .pageFile = "login.mustache",
        .pageScripts = {
            "/static/js/auth.js"
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

}
