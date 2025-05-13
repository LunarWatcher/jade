#pragma once

#include "jade/data/db/Book.hpp"
#include "jade/db/ConnectionPool.hpp"
namespace jade::DBHelper {

std::vector<Book> getBooks(
    pqxx::work& tx,
    int page, std::vector<std::string> searchTags,
    const std::string& titleSearch
);

std::vector<Collection> getCollections(
    pqxx::work& tx,
    int page
);

std::vector<Series> getSeries(
    pqxx::work& tx,
    int page,
    bool includeBooks
);

}
