#include "ContextProvider.hpp"
#include "crow/mustache.h"
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
        assert (serv != nullptr);

        MSessionMiddleware::context& userData = (*serv)->get_context<MSessionMiddleware>(req);


    }

    return ctx;
}

}
