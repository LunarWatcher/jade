#include "Book.hpp"
#include <algorithm>

namespace jade {


crow::json::wvalue Tag::toJson() const {
    return {
        {"Name", name},
        {"ID", tagId}
    };
}

crow::json::wvalue Series::toJson() const {
    std::vector<crow::json::wvalue> bookData;
    std::transform(
       books.begin(), books.end(),
       std::back_inserter(bookData),
       [](auto& book) {
           return book.toJson();
       }
    );

    crow::json::wvalue ret = {
        {"ID", id},
        {"Name", seriesName},
        {"Description", seriesDescription},
        {"Books", bookData}
    };


    return ret;
}

crow::json::wvalue Book::toJson() const {
    std::vector<crow::json::wvalue> tagData;
    std::transform(
       tags.begin(), tags.end(),
       std::back_inserter(tagData),
       [](auto& tag) {
           return tag.toJson();
       }
    );

    crow::json::wvalue ret = {
        {"ID", id},
        {"Title", title},
        {"Description", description},
        {"ISBN", isbn},
        {"Tags", tagData}
    };

    // I really should look into inja
    if (description == "") {
        ret["Description"] = false;
    }
    if (isbn == "") {
        ret["ISBN"] = false;
    }

    return ret;
}

crow::json::wvalue Collection::toJson() const {
    return {
        {"ID", id},
        {"Name", name},
        {"Description", description}
    };
}

}
