#include "BookRequests.hpp"

void jade::from_json(const nlohmann::json& src, BookRequest& dest) {
    src.at("title").get_to(dest.title);
    if (src.contains("isbn")) {
        auto field = src.at("isbn");
        if (field != nullptr) {
            dest.isbn = field.get<std::string>();
        }
    }
    if (src.contains("authors")) {
        auto field = src.at("authors");
        if (field != nullptr) {
            dest.authors = field;
        }
    }
    if (src.contains("tags")) {
        auto field = src.at("tags");
        if (field != nullptr) {
            dest.tags = field;
        }
    }
    if (src.contains("description")) {
        auto field = src.at("description");
        if (field != nullptr) {
            dest.description = field.get<std::string>();
        }
    }
}
