#include "BookRequests.hpp"
#include <sstream>
#include <string>

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

void jade::from_json(const nlohmann::json& src, SearchRequest& dest) {
    if (src.contains("literal") && !src.at("literal").is_null()) {
        dest.literal = src.at("literal");
    }
    if (src.contains("title") && !src.at("title").is_null()) {
        dest.title = src.at("title");
    }
    if (src.contains("tags") && !src.at("tags").is_null()) {
        dest.tags = src.at("tags");
    }
    if (src.contains("authors") && !src.at("authors").is_null()) {
        dest.authors = src.at("authors");
    }
}

namespace jade {

SearchRequest SearchRequest::parse(const std::string_view& raw) {
    SearchRequest out;

    if (raw.size() == 0) {
        return out;
    }
    std::stringstream literal;
    std::stringstream currToken;
    for (size_t i = 0; i < raw.size(); ++i) {
        auto ch = raw.at(i);

        if (ch == '#' && currToken.tellp() == 0) {
            // # at the start of a word
            auto end = raw.find(' ', i);
            if (end == std::string::npos) {
                end = raw.size();
            }
            auto length = end - (i + 1);
            out.tags.push_back(
                std::string(
                    raw.substr(i + 1, length)
                )
            );
            i = end;

        } else if (ch == ':' && currToken.tellp() >= 0) {
            std::string field = currToken.str();
            currToken.str("");
            std::stringstream value;

            if (raw.at(i + 1) == '"') {
                auto end = raw.find('"', i + 2);
                if (end == std::string::npos) {
                    // Glob the rest to avoid exceptions
                    end = raw.size();
                }
                auto length = end - (i + 2);
                value << raw.substr(
                    i + 2,
                    length
                );
                i = end;
            } else {
                // Note a quote block
                auto end = raw.find(' ', i + 1);
                if (end == std::string::npos) {
                    end = raw.size();
                }
                auto length = end - (i + 1);
                value << raw.substr(
                    i + 1,
                    length
                );
                i = end;
            }

            if (field == "author") {
                out.authors.push_back(value.str());
            } else if (field == "title") {
                out.title = value.str();
            } else if (field == "tag") {
                out.tags.push_back(value.str());
            } else {
                // Quietly drop invalid search tokens
                continue;
            }
        } else if (ch == ' ') {
            if (currToken.tellp() == 0) {
                // Space after space, ignore
                continue;
            }
            if (literal.tellp() > 0) {
                literal << " ";
            }
            literal << currToken.str();
            currToken.str("");
        } else {
            currToken << ch;
        }
    }

    if (currToken.tellp() > 0) {
        if (literal.tellp() > 0) {
            literal << " ";
        }
        literal << currToken.str();
    } 
    if (literal.tellp() > 0) {
        out.literal = literal.str();
    }
    return out;
}

}
