#pragma once

#include "jade/health/HealthCheck.hpp"
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

};

}
