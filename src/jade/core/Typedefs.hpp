#pragma once

#include "jade/server/ErrorHandler.hpp"
#include "jade/server/UserContextMiddleware.hpp"
#include <crow.h>
#include <crow/middlewares/cookie_parser.h>

#include <jade/server/BaseMiddlewares.hpp>

namespace jade {

using CrowServer = crow::Crow<
    // Local handlers {{{
    // These two must precede all other handlers, so their 
    // .after_handle is always called. 
    APIErrorHandler, WebError,
    SecurityMetaHeaders,
    crow::CookieParser,

    UserContextM
    // }}}

>;

}
