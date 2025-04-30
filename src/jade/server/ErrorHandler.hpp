#pragma once

#include "crow/middleware.h"
#include "jade/util/Constants.hpp"
#include "nlohmann/json.hpp"

#include <jade/core/ContextProvider.hpp>

namespace jade {

/**
 * Replaces plain errors with full pages (for non-API endpoints), or with a well-defined JSON response wrapping the
 * existing body as a message
 */
class ErrorHandler {
public:
    struct context {};

    void before_handle(crow::request& req, crow::response&, context&) {}
    void after_handle(crow::request& req, crow::response& res, context&) {
        auto message = res.body;
        if (message == "") {
            message = "Error: HTTP " + std::to_string(res.code);
        }
        if (res.code >= 400) {
            if (req.url.starts_with("/api")) {
                // TODO: this isn't _great_ when the message is auto-generated.
                if (res.code == 404)  {
                    message = "invalid endpoint";
                } else if (res.code < 500) {
                    return;
                }
                res.set_header("Content-Type", ContentType::json);
                res.body = nlohmann::json {
                    {"message", message},
                }.dump();
            } else {
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
