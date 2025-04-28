#include "ResponseUtil.hpp"
#include "crow/returnable.h"
#include "jade/util/Constants.hpp"
#include <string>

#include <crow.h>

namespace jade {

JSONResponse::JSONResponse(const std::string& body)
    : crow::returnable(ContentType::json), body(body) {}

JSONResponse::JSONResponse(const nlohmann::json& body)
    : crow::returnable(ContentType::json), body(body.dump()) {}

std::string JSONResponse::dump() const {
    return body;
}

}
