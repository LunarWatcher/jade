#pragma once

#include <string>
#include <nlohmann/json.hpp>

namespace jade {

struct CreateLibRequest {
    std::string location;
};

void from_json(const nlohmann::json& j, CreateLibRequest& o);

}
