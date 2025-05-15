#pragma once

#include "jade/data/db/Book.hpp"
#include "jade/health/HealthCheck.hpp"
#include <optional>
#include <pqxx/pqxx>
#include <filesystem>
#include <shared_mutex>
#include <thread>
#include <unordered_map>

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

class Server;
class Library : public health::HealthCheckable {
private:
    std::string metaCommand = "/usr/bin/ebook-meta";
    /**
     * Supported filetypes. 
     *
     * Currently limited by the filetype support of https://github.com/johnfactotum/foliate-js
     */
    static inline std::vector<std::string> SUPPORTED_EXTENSIONS {
        ".pdf",
        ".epub",
        ".mobi",
        // cbz variants
        ".cbz",
        ".cbr",
        ".cbz",
        ".cbt",
        ".cba",
        ".cb7",
        // KF8 and FB2 omitted because I've never heard about them, nor seen them
    };
    std::unordered_map<int64_t, Source> sources;
    int64_t bookCount;
    
    std::shared_mutex m;

    std::thread runner;
    bool runnerDead = false;

    Server* serv;

    void reindexLibrary(int64_t sourceId, const std::filesystem::path& dir);
    void scan();
    void generateThumbnail(int64_t id, const std::filesystem::path& file);

    /**
     * Utility class that uses the destructor to check if the runner thread abruptly dies.
     */
    struct ThreadCanary {
        Library* host;

        ~ThreadCanary() {
            host->runnerDead = true;
        }
    };
public:
    Library(Server* s, pqxx::connection& conn);

    void initScanProcess();
    
    bool deleteLibrary(Server* serv, int64_t id);
    bool createLibrary(Server* serv, const std::string& location);

    const std::unordered_map<int64_t, Source>& getSources() {
        std::shared_lock l(m);
        return sources; 
    }

    std::vector<health::HealthResult> checkHealth() override;

    /**
     * Returns a vector of Books. 
     *
     * No sorting or filtering is currently supported. The default sort is descending by BookID, meaning (theoretically)
     * the most recently uploaded books are first.
     */
    std::vector<Book> getBooks(size_t page, size_t pagesize = 50);

    /**
     * Returns a vector of Collections.
     *
     * Like getBooks, no sorting or filtering is currently supported. The default sort is descending by CollectionID,
     * meaning the most recently created collections are first
     */
    std::vector<Collection> getCollections(size_t page, size_t pagesize = 50);

    /**
     * Returns a vector of books in a given collection.
     */
    std::vector<Book> getCollection(int64_t collectionId, size_t page, size_t pagesize = 50);

    /**
     * Returns a vector of series without the books in them. Useful for listing series
     */
    std::vector<Series> getAllSeries(size_t page, size_t pagesize = 50);

    /**
     * Returns a single Series with its Books populated.
     */
    Series getSeries(int64_t seriesId, size_t page, size_t pagesize = 50);

    std::optional<Book> getBook(int64_t bookID);

    bool isBookPageNumberValid(size_t page, size_t pagesize, std::optional<int64_t> libraryId = std::nullopt);

};

}
