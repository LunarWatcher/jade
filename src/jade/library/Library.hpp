#pragma once

#include "jade/data/BookRequests.hpp"
#include "jade/data/db/Book.hpp"
#include "jade/health/HealthCheck.hpp"
#include "jade/util/InterruptableThread.hpp"
#include <map>
#include <optional>
#include <pqxx/pqxx>
#include <filesystem>
#include <shared_mutex>
#include <thread>
#include <unordered_map>
#include <vector>

namespace jade {

struct Source {
    std::filesystem::path dir;

    std::filesystem::file_time_type lastUpdate{
        // On Linux, file_clock's epoch is in 2174, which means all timestamps for the next ~150 years are negative, and
        // will therefore fail the update check
        std::chrono::clock_cast<std::chrono::file_clock>(std::chrono::system_clock::time_point())
    };

    int64_t bookCount = 0;

    bool isValid() {
        return std::filesystem::is_directory(dir);
    }

};

enum class BookType {
    PDF,
    EPUB,
    MOBI,

    CB_ZIP,
    CB_RAR,
    CB_GENERIC,
};

template <typename Result>
struct SearchResult {
    int64_t totalResults;
    int64_t totalPages;
    Result res;
};
using BookListResult = SearchResult<std::vector<Book>>;

class Server;
class Library : public health::HealthCheckable {
private:
    std::string metaCommand = "/usr/bin/ebook-meta";
    std::unordered_map<int64_t, Source> sources;
    int64_t bookCount;
    
    std::shared_mutex m;

    InterruptableThread* runner;
    bool runnerDead = false;

    Server* serv;

    void reindexLibrary(int64_t sourceId, const std::filesystem::path& dir);
    void scan();
    void generateThumbnail(int64_t id, const std::filesystem::path& file);
public:
    /**
     * Supported filetypes. 
     *
     * Currently limited by the filetype support of https://github.com/johnfactotum/foliate-js
     */
    static inline const std::map<std::string, BookType> SUPPORTED_EXTENSIONS {
        {".pdf", BookType::PDF},
        {".epub", BookType::EPUB},
        {".mobi", BookType::MOBI},
        // cbz (comic book archive) variants
        {".cbz", BookType::CB_ZIP},
        {".cbr", BookType::CB_RAR},
        // These three do not have a registered content-type in IANA??
        {".cbt", BookType::CB_GENERIC},
        {".cba", BookType::CB_GENERIC},
        {".cb7", BookType::CB_GENERIC},
        // KF8 and FB2 omitted because I've never heard about them, nor seen them
    };

    static inline const std::unordered_map<BookType, std::string> MIMETYPES {
        {BookType::PDF, "application/pdf"},
        {BookType::EPUB, "application/epub+zip"},
        {BookType::MOBI, "application/x-mobipocket-ebook"},
        {BookType::CB_ZIP, "application/vnd.comicbook+zip"},
        {BookType::CB_RAR, "application/vnd.comicbook-rar"},
        {BookType::CB_GENERIC, "application/vnd.comicbook"},
    };

    Library(Server* s, pqxx::connection& conn);
    ~Library() {
        if (runner) {
            runner->kill();
            delete runner;
        }
    }

    void initScanProcess();
    
    bool deleteLibrary(Server* serv, int64_t id);
    bool createLibrary(Server* serv, const std::string& location);

    const std::unordered_map<int64_t, Source>& getSources() {
        std::shared_lock l(m);
        return sources; 
    }

    std::vector<health::HealthResult> checkHealth() override;

    std::optional<BookType> validateExtension(const std::filesystem::path& p);
    bool requestRefresh();

    /**
     * Returns a vector of Books. 
     *
     * No sorting or filtering is currently supported. The default sort is descending by BookID, meaning (theoretically)
     * the most recently uploaded books are first.
     *
     * \param page          The page to request
     * \param pagesize      How many results per page
     * \param searchData    Optional struct with search parameters. Does nothing if the search parameter data objects
     *                      std::nullopt or empty vectors.
     */
    BookListResult getBooks(
        size_t page,
        size_t pagesize = 50,
        std::optional<SearchRequest> searchData = std::nullopt
    );

    std::optional<Book> getBook(int64_t bookID);

    void kill() {
        if (runner) {
            runner->kill();
            delete runner;
            runner = nullptr;
        }
    }

    bool isBookPageNumberValid(size_t page, size_t pagesize, std::optional<int64_t> libraryId = std::nullopt);

};

}
