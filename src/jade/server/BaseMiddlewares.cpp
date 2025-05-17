#include "BaseMiddlewares.hpp"
#include "spdlog/spdlog.h"

#include <crow.h>

namespace jade {


void SecurityMetaHeaders::after_handle(crow::request& req, crow::response& res, context&) {
    res.add_header("Server", "LunarWatcher/Jade " JADE_VERSION);
    res.add_header("X-Frame-Options", "DENY");
    res.add_header("X-Content-Type-Options", "nosniff");
    res.add_header("Referrer-Policy", "strict-origin-when-cross-origin");
    res.add_header("X-XSS-Protection", "0");

    // foliate.js is under the static directory, which means the mimetypes fully rely on support from crow. Crow does
    // not include .mjs, so it's overridden here so foliate.js works.
    if (req.url.ends_with(".mjs") && res.code == 200) {
        res.set_header("Content-Type", "text/javascript");
    } else if (req.url.ends_with(".mjs.map") && res.code == 200) {
        res.set_header("Content-Type", "application/json");
    }

}

}
