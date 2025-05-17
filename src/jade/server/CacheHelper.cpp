#include "CacheHelper.hpp"

namespace jade {

void CacheHelper::after_handle(crow::request& req, crow::response& res, context&) {
    if (res.code == 200) {
        if (req.url.starts_with("/static/js/foliate-js/")) {
            res.set_header("Cache-Control", "public, max-age=2592000");
            res.set_header("Age", "0");
        }
    }
}

}
