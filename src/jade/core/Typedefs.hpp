#pragma once

#include "jade/server/ErrorHandler.hpp"
#include "jade/server/SessionMiddleware.hpp"
#include <crow.h>
#include <crow/middlewares/cookie_parser.h>

#include <jade/server/BaseMiddlewares.hpp>

namespace jade {

using MSessionMiddleware =  jade::SessionMiddleware<jade::InMemoryStorage>;
using CrowServer = crow::Crow<
    // Global handlers {{{
    SecurityMetaHeaders,
    ErrorHandler,
    crow::CookieParser,
    MSessionMiddleware
    // }}}
    // Local handlers {{{



    // }}}

>;


}
