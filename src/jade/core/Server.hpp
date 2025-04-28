#pragma once

#include <crow.h>
#include "jade/conf/ServerConfig.hpp"
#include "jade/core/Typedefs.hpp"
#include "jade/db/ConnectionPool.hpp"

namespace jade {

class Server {
private:
    ServerConfig cfg;
    std::string assetBaseDir;

    void bootstrap();
    void initEndpoints();
    
    void dbMigrations();
public:
    std::shared_ptr<ConnectionPool> pool;

    static inline Server* inst = nullptr;
    CrowServer app;

    Server(
        const std::filesystem::path& confDir
    );
    ~Server();

    void run();

    CrowServer* operator->() {
        return &app;
    }
};

}
