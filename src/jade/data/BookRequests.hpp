#pragma once

#include <string>
#include <vector>
#include <optional>

namespace jade {

struct BookRequest {
    std::string title;
    std::optional<std::string> isbn;
    std::optional<std::string> description;
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

}
