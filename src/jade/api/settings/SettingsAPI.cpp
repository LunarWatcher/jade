#include "SettingsAPI.hpp"

#include "crow/common.h"
#include "jade/core/Macros.hpp"
#include "jade/core/Server.hpp"
#include "jade/data/SettingsRequests.hpp"
#include "jade/server/AuthWall.hpp"
#include "jade/util/ResponseUtil.hpp"

namespace jade {

void Settings::init(Server* server) {
    CROW_ROUTE(server->app, "/api/admin/create-library")
        .methods(crow::HTTPMethod::POST)
        .CROW_MIDDLEWARES(server->app, APINeedsAdmin)
        (JADE_CALLBACK_BINDING(createLibrary));

}

void Settings::createLibrary(Server *server, crow::request &req, crow::response &res) {
    CreateLibRequest r = nlohmann::json::parse(req.body);

    if (r.location.empty() || !r.location.starts_with("/")) {
        res = JSONMessageResponse("Malformed path");
        res.code = crow::BAD_REQUEST;
        res.end();
        return;
    }

    std::filesystem::path p(r.location);
    if (!std::filesystem::is_directory(p)) {
        res = JSONMessageResponse("Path does not exist");
        res.code = crow::BAD_REQUEST;
        res.end();
        return;
    }
    if (!server->lib->createLibrary(server, p.string())) {
        res = JSONMessageResponse("Library already exists");
        res.code = crow::BAD_REQUEST;
        res.end();
        return;
    }

    res = JSONResponse(nlohmann::json{
        {"message", "Success"},
    });
    res.code = 201;
    res.end();
}

}
