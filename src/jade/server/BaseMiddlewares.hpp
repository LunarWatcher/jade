#pragma once

#include <crow.h>

namespace jade {

struct SecurityMetaHeaders {
    struct context {
    };
    void before_handle(crow::request&, crow::response&, context&) {}
    void after_handle(crow::request& req, crow::response& res, context&);
};

}
