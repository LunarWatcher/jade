#pragma once

#include <nlohmann/json.hpp>

namespace jade {

struct BookRequest {
    std::string title;
    std::string isbn;
    std::string description;
    std::vector<std::string> authors;
    std::vector<std::string> tags;
};

void from_json(const nlohmann::json& src, BookRequest& dest);

}
