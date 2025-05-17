#pragma once

#include "crow/json.h"
#include <string>
#include <vector>

namespace jade {

struct Tag {
    std::string name;
    int64_t tagId;

    crow::json::wvalue toJson() const;
};

struct Book;
struct Series {
    int64_t id;
    std::string seriesName;
    std::string seriesDescription;
    std::vector<Book> books;

    crow::json::wvalue toJson() const;
};

struct Book {
    int64_t id;
    std::string title;
    std::string description;
    std::string isbn;
    std::filesystem::path fullPath;
    
    std::vector<Tag> tags;

    crow::json::wvalue toJson() const;
};

/**
 * Describes the metadata for a collection. Unlike series, this does not contain the books, as a collection
 * can potentially have hundreds of books.
 */
struct Collection {
    int64_t id;
    std::string name;
    std::string description;

    crow::json::wvalue toJson() const;
};



}
