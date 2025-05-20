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

struct SearchRequest {
    std::optional<std::string> literal;
    std::optional<std::string> title;
    std::vector<std::string> tags;
    std::vector<std::string> authors;

    static SearchRequest parse(const std::string_view& raw);
};

void from_json(const nlohmann::json& src, BookRequest& dest);
void from_json(const nlohmann::json& src, SearchRequest& dest);

}
