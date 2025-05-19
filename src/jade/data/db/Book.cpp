#include "Book.hpp"
#include "jade/util/Util.hpp"

namespace jade {

crow::json::wvalue Author::toJson() const {
    return {
        {"Name", name},
        {"ID", id}
    };
}

crow::json::wvalue Tag::toJson() const {
    return {
        {"Name", name},
        {"ID", tagId}
    };
}

crow::json::wvalue Book::toJson() const {
    std::stringstream tagData, authorData;
    for (auto& tag : tags) {
        if (tagData.tellp() != 0) {
            tagData << " ";
        }
        tagData << tag.name;
    }
    for (auto& author : authors) {
        if (authorData.tellp() != 0) {
            authorData << " & ";
        }
        authorData << author.name;
    }



    crow::json::wvalue ret = {
        {"ID", id},
        {"Title", title},
        {"Description", description},
        {"ISBN", isbn},
        {"Tags", Util::vec2json<Tag>(tags)},
        {"Authors", Util::vec2json<Author>(authors)},
        {"STags", tagData.str()},
        {"SAuthors", authorData.str()},
    };
    if (tags.empty()) {
        ret["STags"] = false;
    }
    if (authors.empty()) {
        ret["SAuthors"] = false;
    }

    // I really should look into inja
    if (description.empty()) {
        ret["Description"] = false;
    }
    if (isbn.empty()) {
        ret["ISBN"] = false;
    }

    return ret;
}

}
