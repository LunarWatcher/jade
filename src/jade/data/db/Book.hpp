#pragma once

#include "crow/json.h"
#include <string>
#include <vector>

namespace jade {

struct Tag {
    int64_t tagId;
    std::string name;

    crow::json::wvalue toJson() const;
};


struct Author {
    int64_t id;
    std::string name;

    crow::json::wvalue toJson() const;
};

struct Book {
    int64_t id;
    std::string title;
    std::string description;
    std::string isbn;
    /**
     * \warning Not included in toJson for security reasons
     */
    std::filesystem::path fullPath;
    
    std::vector<Tag> tags;
    std::vector<Author> authors;

    crow::json::wvalue toJson() const;
};



}
