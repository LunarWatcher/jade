#pragma once

#include "crow/returnable.h"
#include "jade/util/Constants.hpp"

#include <rfl/json.hpp>
#include <rfl.hpp>

namespace jade {

struct MessageResponse {
    std::string message;
};

/**
 * Utility class to convert JSON responses to string, and set the content type to JSON
 */
class JSONResponse : public crow::returnable {
private:
    std::string body;
public:
    template <typename T>
    JSONResponse(const T& obj) :
        crow::returnable(ContentType::json),
        body(rfl::json::write(obj)) {

    }

    std::string dump() const override;
};

}
