#pragma once

#include "crow/json.h"
#include <filesystem>
#include <string>
#include <vector>

namespace jade {

struct Tag {
    std::string name;
    int64_t tagId;

    crow::json::wvalue toJson();
};

struct Book;
struct Series {
    int64_t id;
    std::string seriesName;
    std::string seriesDescription;
    std::vector<Book> books;

    crow::json::wvalue toJson();
};

struct Book {
    int64_t id;
    std::string title;
    std::string description;
    std::string isbn;
    
    std::vector<Tag> tags;

    crow::json::wvalue toJson();
};

/**
 * Describes the metadata for a collection. Unlike series, this does not contain the books, as a collection
 * can potentially have hundreds of books.
 */
struct Collection {
    int64_t id;
    std::string name;
    std::string description;

    crow::json::wvalue toJson();
};



}
