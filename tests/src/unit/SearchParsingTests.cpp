#include "jade/data/BookRequests.hpp"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Base cases for search", "[SearchParsing]") {
    SECTION("Metatests") {
        auto empty = jade::SearchRequest::parse("");
        REQUIRE(!empty.literal.has_value());
        REQUIRE(!empty.title.has_value());
        REQUIRE(empty.tags.empty());
        REQUIRE(empty.authors.empty());
    }
    SECTION("Basic search string") {
        auto q = jade::SearchRequest::parse("some search query");
        REQUIRE(q.literal.has_value());
        REQUIRE(*q.literal == "some search query");
        REQUIRE(!q.title.has_value());
        REQUIRE(q.tags.empty());
        REQUIRE(q.authors.empty());
    }

    SECTION("Qualifiers without quotes") {
        auto q = jade::SearchRequest::parse("tag:SomeTag author:SomeAuthor title:SomeTitle");
        INFO(q.literal.value_or("<literal !has_value>"));
        REQUIRE(!q.literal.has_value());
        REQUIRE(q.title.has_value());
        REQUIRE(q.tags.size() == 1);
        REQUIRE(q.authors.size() == 1);

        REQUIRE(*q.title == "SomeTitle");
        REQUIRE(q.tags.at(0) == "SomeTag");
        REQUIRE(q.authors.at(0) == "SomeAuthor");
    }

    SECTION("Literals mixed with qualifiers") {
        auto q = jade::SearchRequest::parse("mixed tag:SomeTag literals title:SomeTitle");
        REQUIRE(q.literal.has_value());
        REQUIRE(q.title.has_value());
        REQUIRE(!q.tags.empty());

        REQUIRE(q.literal.value() == "mixed literals");
        REQUIRE(q.title.value() == "SomeTitle");
        REQUIRE(q.tags.at(0) == "SomeTag");
    }

    SECTION("Multiple stringified qualifiers") {
        auto q = jade::SearchRequest::parse("title:\"This is a title\" author:\"Rick Astley\"");
        INFO(q.literal.value_or("<literal !has_value>"));
        REQUIRE(!q.literal.has_value());
        REQUIRE(q.title.has_value());
        REQUIRE(q.authors.size() == 1);

        REQUIRE(q.title.value() == "This is a title");
        REQUIRE(q.authors.at(0) == "Rick Astley");

    }
    SECTION("Complex mixed scenario") {
        auto q = jade::SearchRequest::parse("Literal search #MyTag title:\"This is a title\" author:\"Rick Astley\"");
        INFO(q.literal.value_or("<literal !has_value>"));
        REQUIRE(q.literal.has_value());
        REQUIRE(q.title.has_value());
        REQUIRE(q.tags.size() == 1);
        REQUIRE(q.authors.size() == 1);

        REQUIRE(q.literal.value() == "Literal search");
        REQUIRE(q.title.value() == "This is a title");
        REQUIRE(q.tags.at(0) == "MyTag");
        REQUIRE(q.authors.at(0) == "Rick Astley");
    }
    SECTION("Individual edge-case checks") {
        auto tag = jade::SearchRequest::parse("#MyTag");
        REQUIRE(tag.tags.size() == 1);
        REQUIRE(tag.tags.at(0) == "MyTag");

        auto quotedQual = jade::SearchRequest::parse("tag:\"My Invalid Tag\"");
        REQUIRE(quotedQual.tags.size() == 1);
        REQUIRE(quotedQual.tags.at(0) == "My Invalid Tag");
    }
    SECTION("Invalid fields") {
        auto q = jade::SearchRequest::parse("fjdgkljdkfglsfdskjlfds:\"String value\" literal");

        REQUIRE(q.literal.has_value());
        REQUIRE(!q.title.has_value());
        REQUIRE(q.authors.empty());
        REQUIRE(q.tags.empty());

        REQUIRE(q.literal.value() == "literal");
    }
}
