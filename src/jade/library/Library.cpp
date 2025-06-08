#include "Library.hpp"
#include "jade/data/BookRequests.hpp"
#include "jade/health/HealthCheck.hpp"
#include "jade/util/InterruptableThread.hpp"
#include "spdlog/spdlog.h"
#include <cctype>
#include <filesystem>
#include <iterator>

#include <stc/Environment.hpp>
#include <stc/StringUtil.hpp>

#include <jade/core/Server.hpp>
#include <string_view>

namespace jade {

Library::Library(Server* s, pqxx::connection& conn) : serv(s) {
    pqxx::work w(conn);
    auto libraries = w.query<int64_t, std::string>("SELECT LibraryID, Location FROM Jade.Libraries");

    std::transform( // NOLINT - clang-tidy whines about a potential bug in libpqxx, which I can't do anything about
                    // anyway
        libraries.begin(), libraries.end(), 
        std::inserter(sources, sources.begin()),
        [](const auto& row) -> std::pair<int64_t, Source> {
            return {
                std::get<0>(row),
                Source {
                    std::get<1>(row)
                }
            };
        }
    );

    if (sources.empty()) {
        spdlog::warn("No active libraries");
    }

    w.abort();
}

void Library::scan() {

    spdlog::debug("Index starting");
    for (auto& [id, source] : sources) {
        auto& dir = source.dir;
        if (!std::filesystem::exists(dir)) {
            spdlog::error("Index failure: {} does not exist (bad mount?)", dir.string());
            continue;
        }
        // Under ext4, adding or updating a file results in last_write_time being updated.
        if (auto newUpdate = std::filesystem::last_write_time(dir);
            newUpdate > source.lastUpdate)
        {
            source.lastUpdate = newUpdate;

            reindexLibrary(id, dir);
        } else {
            spdlog::debug("{} is up to date", source.dir.string());
        }
    }

    bookCount = 0;
    serv->pool->acquire<void>([this](auto& conn) {
        pqxx::work w(conn);

        for (auto& [id, source] : sources) {
            auto res = w.query1<int64_t>(
                "SELECT COUNT(*) FROM Jade.Books WHERE LibraryID = $1",
                pqxx::params {
                    id
                }
            );
            source.bookCount = std::get<0>(res);
            bookCount += source.bookCount;
        }
        w.abort();
    });

    if (sources.empty()) {
        spdlog::info("Scanner: No libraries exist to scan");
    } else {
        spdlog::debug("Index complete");
    }
}

void Library::initScanProcess() {
    runner = new InterruptableThread {
        std::bind(&Library::scan, this),
        std::chrono::minutes(60)
    };
}

bool Library::createLibrary(Server* serv, const std::string& location) {
    return serv->pool->acquire<bool>([&](auto& conn) {
        pqxx::work w(conn);

        auto exists = w.query1<bool>(
            "SELECT EXISTS(SELECT 1 FROM Jade.Libraries WHERE Location = $1)", 
            pqxx::params {location}
        );

        if (std::get<0>(exists)) {
            w.abort();
            return false;
        }

        auto res = w.exec(R"(INSERT INTO Jade.Libraries (LibraryID, Location) VALUES (DEFAULT, $1) RETURNING LibraryID)",
            pqxx::params {
                location
            }
        ).one_row();
        auto id = res.at(0).as<int64_t>();
        spdlog::info("LibraryID {} (loc: {}) created", id, location);

        w.commit();
        std::unique_lock l(m);
        sources[id] = Source {
            location
        };
        return true;
    });
}

std::vector<health::HealthResult> Library::checkHealth() {
    std::vector<health::HealthResult> out;

    std::vector<health::HealthCheck> sourceChecks;

    {
        std::shared_lock l(m);
        for (auto& [id, source] : this->sources) {
            auto isValid = source.isValid();
            sourceChecks.push_back({
                isValid,
                source.dir.string(),
                isValid ? "" : "Path does not exist, or is not a directory"
            });
        }
    }

    sourceChecks.push_back({
        !runnerDead,
        "Library scanner thread",
        runnerDead ? "Defunct" : "Running"
    });

    out.push_back({
        "Library sources",
        sourceChecks
    });

    return out;
}

void Library::reindexLibrary(int64_t sourceId, const std::filesystem::path& dir) {
    serv->pool->acquire<void>([&](auto& conn) {
        pqxx::work w(conn);
        for (auto& file : std::filesystem::directory_iterator(dir)) {
            auto path = std::filesystem::relative(
                file.path(),
                dir
            );

            
            if (!validateExtension(path).has_value()) {
                spdlog::warn("{} is not supported (invalid extension)", path.string());
                continue;
            }

            if (std::get<0>(
                w.query1<bool>(
                    "SELECT EXISTS(SELECT 1 FROM Jade.Books WHERE FileName = $1)", 
                    pqxx::params {
                        path.string()
                    }
                )
            ) == false) {
                spdlog::info("New book detected in {}: {}", dir.string(), path.string());

                auto strPath = (dir/path).string();
                int code;
                auto output = stc::syscommand(
                    std::vector {
                        metaCommand.c_str(),
                        strPath.c_str()
                    },
                    &code
                );

                std::string title;
                if (code == 0) {
                    auto lines = stc::string::split(output, '\n');

                    for (auto& line : lines) {
                        if (line.starts_with("Title")) {
                            // + 2 is required to exclude the colon, and exclude the space immediately after it
                            title = line.substr(line.find(':') + 2);
                            break;
                        }
                    }
                } else {
                    spdlog::error("ebook-meta failed (code = {}): {}", code, output);
                }

                auto res = w.exec(
                    R"(INSERT INTO Jade.Books (FileName, LibraryID, Title) VALUES ($1, $2, $3) RETURNING BookID;)",
                    
                    pqxx::params {
                        path.string(),
                        sourceId,
                        title
                    }
                );

                auto insertedId = res.at(0).as<int64_t>();

                generateThumbnail(std::get<0>(insertedId), dir / path);
            }

        }

        std::vector<int64_t> toRemove;
        for (auto [bookId, filename] : w.stream<int64_t, std::string_view>(
            "SELECT BookID, FileName FROM Jade.Books WHERE LibraryID = " + std::to_string(sourceId)
            //pqxx::params {
                //sourceId
            //}
        )) {
            if (!std::filesystem::exists(dir / filename)) {
                auto thumbnail = serv->getConfig().thumbnailCacheDir / (std::to_string(bookId) + ".jpg");
                if (std::filesystem::exists(thumbnail)) {
                    std::filesystem::remove(thumbnail);
                }
                toRemove.push_back(bookId);
            }
        }

        for (const auto& bookId : toRemove) {
            w.exec("DELETE FROM Jade.Books WHERE BookID = $1", pqxx::params {
                bookId
            });
            spdlog::info("Book {} no longer exists and has been removed", bookId);
        }

        w.commit();
    });
}

void Library::generateThumbnail(int64_t id, const std::filesystem::path& file) {
    auto output = serv->getConfig().thumbnailCacheDir / (std::to_string(id) + ".jpg");

    int code;
    stc::syscommand(std::vector {
        metaCommand.c_str(),
        file.string().c_str(),
        ("--get-cover=" + output.string()).c_str()
    }, &code);
    if (code != 0) {
        spdlog::error("Failed to generate thumbnail for {} (error code {})", file.string(), code);
    } else {
        spdlog::debug("Generated thumbnail for {}", file.string());
    }

}

BookListResult Library::getBooks(size_t page, size_t pagesize, std::optional<SearchRequest> searchData) {
    return serv->pool->acquire<BookListResult>([&](auto& conn) -> BookListResult {
        pqxx::work w(conn);
        std::vector<Book> out;

        pqxx::params p {
            pagesize,
            page * pagesize
        };
        std::stringstream query;
        query << R"(SELECT 
                COUNT(*) OVER(),
                BookID,
                Title,
                COALESCE(Description, ''), 
                COALESCE(ISBN, '')
            FROM Jade.Books
        )";

        // TODO: This is cursed, probably inefficient as fuck, and I fucking hate it, but this is as good as it's going
        // to get for now. This needs to be replaced by a proper search index rather than these absolutely jank SQL
        // queries
        //
        // (That said, holy fuck, I cannot believe it fucking works!)
        if (
            searchData.has_value()
            && (
                searchData->literal.has_value()
                || searchData->title.has_value()
                || !searchData->tags.empty()
                || !searchData->authors.empty()
            )
        ) {
            query << "WHERE\n";

            bool hasField = false;
            size_t runningIdx = 2;
            if (searchData->literal.has_value()) {
                hasField = true;
                ++runningIdx;
                query
                    << "(Title ILIKE '%' || $" << runningIdx << " || '%'"
                    << "OR Description ILIKE '%' || $" << runningIdx << " || '%')\n";
                p.append(searchData->literal.value());
            }

            if (searchData->title.has_value()) {
                if (hasField) {
                    query << " AND ";
                }
                hasField = true;
                query << "Title ILIKE '%' || $" << ++runningIdx << " || '%'\n";
                p.append(*searchData->title);
            }
            if (!searchData->tags.empty()) {
                if (hasField) {
                    query << " AND ";
                }
                hasField = true;
                query << std::format(R"(
                Jade.Books.BookID IN (
                    SELECT r.BookID
                    FROM Jade.BookTags r
                    JOIN Jade.Tags t on r.TagID = t.TagID
                    WHERE t.TagName = ANY(${0}) AND Jade.Books.BookID = r.BookID
                    GROUP BY r.BookID
                    HAVING COUNT(DISTINCT t.TagName) = CARDINALITY(${0})
                )
                )", ++runningIdx);
                p.append(searchData->tags);
            }
            if (!searchData->authors.empty()) {
                if (hasField) {
                    query << " AND ";
                }
                hasField = true;
                query << std::format(R"(
                Jade.Books.BookID IN (
                    SELECT r.BookID
                    FROM Jade.BookAuthors r
                    JOIN Jade.Authors t on r.AuthorID = t.AuthorID
                    WHERE t.AuthorName ILIKE ANY(${0}) AND Jade.Books.BookID = r.BookID
                    GROUP BY r.BookID
                    HAVING COUNT(DISTINCT t.AuthorName) = CARDINALITY(${0})
                )
                )", ++runningIdx);
                p.append(searchData->authors);
            }
        }

        query << R"(
            ORDER BY BookID DESC
            LIMIT $1
            OFFSET $2)";
        spdlog::debug("Running {}", query.str());
        auto rows = w.query<
            int64_t, int64_t, std::string_view, std::string_view, std::string_view
        >(query.str(), p);

        int64_t totalResults = 0;
        if (rows.begin() != rows.end()) {
            totalResults = std::get<0>(*rows.begin());
            for (auto& [_, id, title, description, isbn] : rows) {
                
                Book b {
                    .id = id,
                    .title = std::string{ title },
                    .description = std::string{ description },
                    .isbn = std::string{ isbn },
                };

                // TODO: This feels extremely inefficient
                auto tags = w.query<int64_t, std::string_view>(R"(
                Select Jade.BookTags.TagID, Jade.Tags.TagName
                FROM Jade.BookTags
                INNER JOIN Jade.Tags ON Jade.Tags.TagID = Jade.BookTags.TagID
                WHERE Jade.BookTags.BookID = $1;
                )", pqxx::params {
                    id
                });
                auto authors = w.query<int64_t, std::string_view>(R"(
                Select Jade.BookAuthors.AuthorID, Jade.Authors.AuthorName
                FROM Jade.BookAuthors
                INNER JOIN Jade.Authors ON Jade.Authors.AuthorID = Jade.BookAuthors.AuthorID
                WHERE Jade.BookAuthors.BookID = $1;
                )", pqxx::params {
                    id
                });

                for (auto& [id, name] : tags) {
                    b.tags.push_back(Tag { id, std::string(name) });
                }
                for (auto& [id, name] : authors) {
                    b.authors.push_back(Author { id, std::string(name) });
                }

                out.push_back(b);
            }
        }
        w.abort();
        return {
            .totalResults = totalResults,
            .totalPages = (int64_t) std::ceil((double) totalResults / (double) pagesize),
            .res = out
        };
    });
}

std::optional<Book> Library::getBook(int64_t bookID) {
    return serv->pool->acquire<std::optional<Book>>([&](auto& conn) -> std::optional<Book> {
        pqxx::work w(conn);

        auto row = w.query01<
            std::string_view, std::string_view, std::string_view, std::string_view, int64_t
        >(
            R"(
            SELECT 
                Title,
                COALESCE(Description, ''), 
                COALESCE(ISBN, ''),
                FileName,
                LibraryID
            FROM Jade.Books
            WHERE BookID = $1
            )",
            pqxx::params {
                bookID
            }
        );

        if (!row.has_value()) {
            return std::nullopt;
        }

        Book b {
            bookID,
            std::string { std::get<0>(*row) },
            std::string { std::get<1>(*row) },
            std::string { std::get<2>(*row) },
            std::string { sources.at(std::get<4>(*row)).dir / std::get<3>(*row) },
        };

        // TODO: This feels extremely inefficient
        auto tags = w.query<int64_t, std::string_view>(R"(
        Select Jade.BookTags.TagID, Jade.Tags.TagName
        FROM Jade.BookTags
        INNER JOIN Jade.Tags ON Jade.Tags.TagID = Jade.BookTags.TagID
        WHERE Jade.BookTags.BookID = $1
        )", pqxx::params {
            bookID
        });
        auto authors = w.query<int64_t, std::string_view>(R"(
        Select Jade.BookAuthors.AuthorID, Jade.Authors.AuthorName
        FROM Jade.BookAuthors
        INNER JOIN Jade.Authors ON Jade.Authors.AuthorID = Jade.BookAuthors.AuthorID
        WHERE Jade.BookAuthors.BookID = $1
        )", pqxx::params {
            bookID
        });

        for (auto& [id, name] : tags) {
            b.tags.push_back(Tag { id, std::string(name) });
        }
        for (auto& [id, name] : authors) {
            b.authors.push_back(Author { id, std::string(name) });
        }

        return b;
    });
}

std::optional<BookType> Library::validateExtension(const std::filesystem::path& p) {
    auto ext = p.extension().string();
    std::transform(ext.cbegin(), ext.cend(), ext.begin(), [](auto ch) { return std::tolower(ch); });

    auto it = SUPPORTED_EXTENSIONS.find(ext);
    if (it == SUPPORTED_EXTENSIONS.end()) {
        return std::nullopt;
    }

    return it->second;
}

bool Library::requestRefresh() {
    if (!runner) { return false; }
    return runner->interrupt();
}

bool Library::isBookPageNumberValid(size_t page, size_t pagesize, std::optional<int64_t> libraryId) {
    auto offset = page * pagesize;
    if (libraryId.has_value()) {
        // TODO: >=?
        return sources.at(*libraryId).bookCount > offset;
    }

    // TODO: >=?
    return bookCount > offset;
}

}
