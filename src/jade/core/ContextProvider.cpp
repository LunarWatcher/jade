#include "ContextProvider.hpp"
#include "crow/mustache.h"
#include <jade/core/Server.hpp>

#include <jade/server/SessionMiddleware.hpp>

namespace jade {

crow::mustache::context ContextProvider::buildBaseContext(
    int contextScope,
    const crow::request& req,
    PageContext& baseCtx,
    Server* serv
) {
    crow::mustache::context ctx = baseCtx.getContext();
    if ((contextScope & USER) != 0) {
        spdlog::debug("Including user details");
        _detail::buildUserContext(ctx, req, serv);
    }
    if ((contextScope & LIBRARIES) != 0) {
        spdlog::debug("Including library details");
        _detail::buildLibraryContext(ctx, serv);
    }

    return ctx;
}

namespace ContextProvider {

void _detail::buildUserContext(
    crow::mustache::context& ctx,
    const crow::request& req,
    Server* serv
) {

    assert (serv != nullptr);

    const MSessionMiddleware::context& userData = (*serv)->get_context<MSessionMiddleware>(req);
    if (userData.data != nullptr && userData.data->user.has_value()) {
        auto& user = userData.data->user.value();
        ctx["User"] = {
            {"Name", user.username},
            {"ID", (int64_t) user.userId},
            {"IsAdmin", user.isAdmin}
        };
    }
}

void _detail::buildLibraryContext(
    crow::mustache::context& ctx,
    Server* serv
) {
    assert (serv != nullptr);

    auto& library = *serv->lib;

    std::vector<crow::json::wvalue> out;
    for (const auto& [id, source] : library.getSources()) {
        out.push_back({
            {"ID", (int64_t) id},
            {"Location", source.dir.c_str()},
        });
    }

    ctx["Library"] = std::move(out);
}

}

}
