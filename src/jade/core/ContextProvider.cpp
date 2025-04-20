#include "ContextProvider.hpp"
#include "crow/mustache.h"
#include "jade/server/UserContextMiddleware.hpp"
#include <jade/core/Server.hpp>

namespace jade {

crow::mustache::context ContextProvider::buildBaseContext(
    int contextScope,
    crow::request& req,
    PageContext& baseCtx,
    Server* serv
) {
    crow::mustache::context ctx = baseCtx.getContext();
    if (contextScope & USER) {
        UserContextM::context& userData = (*serv)->get_context<UserContextM>(req);

        if (userData.user.has_value()) {

        }
    }

    return ctx;
}

}
