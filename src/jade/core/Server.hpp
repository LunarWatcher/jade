#pragma once

#include "crow/mustache.h"
#include "jade/conf/ServerConfig.hpp"
#include "jade/core/Typedefs.hpp"

namespace jade {

class Server {
private:
    ServerConfig cfg;
    std::string assetBaseDir;


    void bootstrap();
    void initEndpoints();
public:
    CrowServer app;

    Server(
        const std::filesystem::path& confDir
    );

    void run();

    crow::mustache::context buildCommonContext();

    CrowServer* operator->() {
        return &app;
    }
};

}
