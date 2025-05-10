#pragma once

#include "crow/returnable.h"
#include <nlohmann/json.hpp>

namespace jade {

/**
 * Utility class to convert JSON responses to string, and set the content type to JSON
 */
class JSONResponse : public crow::returnable {
private:
    std::string body;
public:
    JSONResponse(const std::string& body);
    // Required to avoid conversion errors. Could require explicit `std::string` conversion outside, but it's easier to
    // do it this way
    JSONResponse(const char* c) : JSONResponse(std::string(c)) {}
    JSONResponse(const nlohmann::json& body);

    std::string dump() const override;
};

class JSONMessageResponse : public JSONResponse {
public:
    JSONMessageResponse(const std::string& message) : JSONResponse(
        nlohmann::json {
            {"message", message},
        }
    ) {}
};

}
