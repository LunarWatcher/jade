#pragma once

#include "crow/middleware.h"
#include "nlohmann/json.hpp"

#include <jade/core/ContextProvider.hpp>

namespace jade {

template <bool IsAPI>
class ErrorHandler : public crow::ILocalMiddleware {
public:
    struct context {};

    void before_handle(crow::request&, crow::response&, context&) {}
    void after_handle(crow::request& req, crow::response& res, context&) {
        auto message = res.body;
        if (message == "") {
            message = "Error: HTTP " + std::to_string(res.code);
        }
        if (res.code >= 400) {
            if constexpr (IsAPI) {
                res.set_header("Content-Type", "application/json");
                res.body = nlohmann::json {
                    {"message", message},
                }.dump();
            } else {
                res.set_header("Content-Type", "text/html");
                static ContextProvider::PageContext ctx {
                    .pageTitle = "Error",
                    .pageDescription = "An error happened",
                    .pageFile = "error.mustache"
                };

                auto context = ContextProvider::buildBaseContext(0, req, ctx, nullptr);
                context["ErrorCode"] = res.code;
                context["ErrorMessage"] = message;

                req.body = ContextProvider::getBaseTemplate()
                    .render_string(context);
            }
        }
    }
};

using APIErrorHandler = ErrorHandler<true>;
using WebError = ErrorHandler<false>;

}
