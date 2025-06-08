#include "BaseMiddlewares.hpp"

#include <crow.h>

namespace jade {


void SecurityMetaHeaders::after_handle(crow::request& req, crow::response& res, context&) {
    res.add_header("Server", "LunarWatcher/Jade " JADE_VERSION);
    res.add_header("X-Frame-Options", "DENY");
    res.add_header("X-Content-Type-Options", "nosniff");
    res.add_header("Referrer-Policy", "strict-origin-when-cross-origin");
    res.add_header("X-XSS-Protection", "0");
    res.add_header(
        "Content-Security-Policy",
        "default-src 'self'; "
        "img-src 'self' blob:; "
        // TODO: This is insecure, but foliate appears to need it. See:
        //      https://github.com/johnfactotum/foliate-js/issues/64
        "style-src 'self' 'unsafe-inline' blob:; "
        "frame-ancestors 'none'; "
        "frame-src 'self' blob:; "
    );

    // foliate.js is under the static directory, which means the mimetypes fully rely on support from crow. Crow does
    // not include .mjs, so it's overridden here so foliate.js works.
    if (res.code == 200) {
        if (req.url.ends_with(".mjs")) {
            res.set_header("Content-Type", "text/javascript");
        } else if (req.url.ends_with(".mjs.map")) {
            res.set_header("Content-Type", "application/json");
        } else if (req.url.ends_with(".epub")) {
            res.set_header("Content-Type", "application/epub+zip");
        }
    }

}

}
