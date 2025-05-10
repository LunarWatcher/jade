#pragma once

#include "jade/server/SessionMiddleware.hpp"
#include <pqxx/pqxx>
#include <filesystem>
#include <shared_mutex>
#include <thread>
#include <unordered_map>

namespace jade {

struct Source {
    std::filesystem::path dir;

    std::chrono::system_clock::time_point lastUpdate;
};

class Server;

class Library {
private:
    std::unordered_map<int64_t, Source> sources;
    std::shared_mutex m;

    std::thread runner;

    void scan();
public:
    Library(pqxx::connection& conn);

    void initScanProcess();
    
    bool deleteLibrary(Server* serv, int64_t id);
    bool createLibrary(Server* serv, const std::string& location);

    const std::unordered_map<int64_t, Source>& getSources() {
        std::shared_lock l(m);
        return sources; 
    }

};

}
