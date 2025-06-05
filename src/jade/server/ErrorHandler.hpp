#pragma once

#include "jade/util/Constants.hpp"
#include "jade/util/ResponseUtil.hpp"

#include <jade/core/ContextProvider.hpp>
#include <rfl.hpp>
#include <rfl/json.hpp>

namespace jade {

/**
 * Replaces plain errors with full pages (for non-API endpoints), or with a well-defined JSON response wrapping the
 * existing body as a message.
 *
 * Note that for API endpoints, the replacement mechanics only happen for 404s and 5xx errors. It's assumed that
 * endpoints error handle with first-class JSON.
 */
class ErrorHandler {
public:
    struct context {};

    void before_handle(crow::request& req, crow::response&, context&) {}
    void after_handle(crow::request& req, crow::response& res, context&) {
        auto message = res.body;
        if (res.code >= 400) {
            if (req.url.starts_with("/api")) {
                // TODO: this isn't _great_ when the message is auto-generated.
                if (res.code == 404 && message == "")  {
                    message = "invalid endpoint";
                } else if (res.code < 500) {
                    return;
                }
                // can't use `res = JSONResponse` here without invalidating the `code`, and any potential other headers.
                // This avoids fucking with that
                res.set_header("Content-Type", ContentType::json);
                res.body = rfl::json::write(MessageResponse{ message });
            } else {
                if (message == "") {
                    message = "Error: HTTP " + std::to_string(res.code);
                }
                static ContextProvider::PageContext ctx {
                    .pageTitle = "Error",
                    .pageDescription = "An error happened",
                    .pageFile = "error.mustache"
                };

                auto context = ContextProvider::buildBaseContext(0, req, ctx, nullptr);
                context["ErrorCode"] = res.code;
                context["ErrorMessage"] = message;

                res.set_header("Content-Type", ContentType::html);
                res.body = ContextProvider::getBaseTemplate()
                    .render_string(context);
            }
        }
    }
};

}
