#pragma once

#include "crow/middleware.h"
#include "jade/server/SessionMiddleware.hpp"
#include "jade/util/ResponseUtil.hpp"

namespace jade {

template <bool NeedsAdmin, bool IsAPI>
struct RequireAuthedUserMiddleware : public crow::ILocalMiddleware {
    struct context {

    };
    template <typename AllContext>
    void before_handle(crow::request& req, crow::response& res, context& ctx, AllContext& globalCtx) {
        MSessionMiddleware::context& user = globalCtx.template get<MSessionMiddleware>();
        if (!user.data || !user.data->user) {
            // Not authenticated
            if constexpr (IsAPI) {
                res = JSONMessageResponse("This endpoint requires authentication.");
                res.code = crow::FORBIDDEN;
            } else {
                res.redirect("/auth/login.html");
            }
            res.end();
        } else if constexpr (NeedsAdmin) {
            if (!user.data->user->isAdmin) {
                // Authed, but not an admin
                res = JSONMessageResponse("You lack the permission required to see this.");
                res.code = crow::FORBIDDEN;
                res.end();
            }
        }
    }
    void after_handle(crow::request& req, crow::response&, context& ctx) {}
};

using NeedsAdmin = RequireAuthedUserMiddleware<true, false>;
using NeedsAuthed = RequireAuthedUserMiddleware<false, false>;
using APINeedsAdmin = RequireAuthedUserMiddleware<true, true>;
using APINeedsAuthed = RequireAuthedUserMiddleware<false, true>;

}
