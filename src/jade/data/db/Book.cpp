#include "Book.hpp"
#include <algorithm>

namespace jade {


crow::json::wvalue Tag::toJson() {
    return {
        {"Name", name},
        {"ID", tagId}
    };
}

crow::json::wvalue Series::toJson() {
    std::vector<crow::json::wvalue> bookData;
    std::transform(
       books.begin(), books.end(),
       std::back_inserter(bookData),
       [](auto& book) {
           return book.toJson();
       }
    );

    return {
        {"ID", id},
        {"Name", seriesName},
        {"Description", seriesDescription},
        {"Books", bookData}
    };

}

crow::json::wvalue Book::toJson() {
    std::vector<crow::json::wvalue> tagData;
    std::transform(
       tags.begin(), tags.end(),
       std::back_inserter(tagData),
       [](auto& tag) {
           return tag.toJson();
       }
    );

    return {
        {"ID", id},
        {"Title", title},
        {"Description", description},
        {"ISBN", isbn},
        {"Tags", tagData}
    };

}

crow::json::wvalue Collection::toJson() {
    return {
        {"ID", id},
        {"Name", name},
        {"Description", description}
    };
}

}
