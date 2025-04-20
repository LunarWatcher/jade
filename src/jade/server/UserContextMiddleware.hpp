#pragma once

#include "crow/middleware.h"
#include "jade/data/User.hpp"

namespace jade {

class UserContextM : public crow::ILocalMiddleware {
public:
    struct context {
        std::optional<User> user;
    };

    void before_handle(crow::request& req, crow::response& res, context& ctx);
    void after_handle(crow::request&, crow::response&, context&) {}
};

template <bool RequireIsAdmin = false>
class UserRequiredM : public crow::ILocalMiddleware {
public:
    struct context {

    };

    template <class Contexts>
    void before_handle(crow::request& req, crow::response& res, context& ctx, Contexts& extras) {
        UserContextM::context& userContext = extras.template get<UserContextM>();

        if (!userContext.user.has_value()) {
            res.end();
        }

        if constexpr (RequireIsAdmin && userContext.user->isAdmin == false) {
            res.end();
        }
    }
    void after_handle(crow::request&, crow::response&, context&) {}
};

}
