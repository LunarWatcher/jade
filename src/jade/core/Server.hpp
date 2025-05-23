#pragma once

#include <crow.h>
#include "jade/conf/ServerConfig.hpp"
#include "jade/core/Typedefs.hpp"
#include "jade/db/ConnectionPool.hpp"
#include "jade/health/HealthCore.hpp"
#include "jade/library/Library.hpp"

namespace jade {

struct Flags {
    bool debugCrow = false;
    bool purgeDatabase = false;
};

class Server {
private:
    ServerConfig cfg;
    std::string assetBaseDir;

    void bootstrap();
    void initEndpoints();
    
    void dbMigrations();
    void initHealth();
public:
    std::shared_ptr<ConnectionPool> pool;
    std::shared_ptr<Library> lib;
    health::HealthCore healthCore;
    Flags runtimeConfig;

    static inline Server* inst = nullptr;
    CrowServer app;

    Server(
        const std::filesystem::path& confDir,
        Flags runtimeConfig
    );
    ~Server();

    void run();

    const ServerConfig& getConfig() {
        return cfg;
    }

    CrowServer* operator->() {
        return &app;
    }
};

}
