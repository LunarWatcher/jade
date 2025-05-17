#pragma once

#include "jade/server/AuthWall.hpp"
#include "jade/server/CacheHelper.hpp"
#include "jade/server/ErrorHandler.hpp"
#include "jade/server/SessionMiddleware.hpp"
#include <crow.h>
#include <crow/middlewares/cookie_parser.h>

#include <jade/server/BaseMiddlewares.hpp>

namespace jade {

using CrowServer = crow::Crow<
    // Global handlers {{{
    CacheHelper,
    SecurityMetaHeaders,
    crow::CookieParser,
    MSessionMiddleware,
    ErrorHandler,
    // }}}
    // Local handlers {{{

    NeedsAdmin,
    NeedsAuthed,
    APINeedsAdmin,
    APINeedsAuthed

    // }}}

>;


}
