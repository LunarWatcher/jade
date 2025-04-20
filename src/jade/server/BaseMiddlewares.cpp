#include "BaseMiddlewares.hpp"

#include <crow.h>

namespace jade {


void SecurityMetaHeaders::after_handle(crow::request&, crow::response& res, context&) {
    res.add_header("Server", "LunarWatcher/Jade " JADE_VERSION);
    res.add_header("X-Frame-Options", "DENY");
    res.add_header("X-Content-Type-Options", "nosniff");
    res.add_header("Referrer-Policy", "strict-origin-when-cross-origin");
    res.add_header("X-XSS-Protection", "0");

}

}
