#include "Server.hpp"
#include "crow/app.h"
#include "jade/web/WebProvider.hpp"

#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

namespace jade {

Server::Server(const std::filesystem::path& confDir) {
    std::ifstream f(confDir / "config.json");

    if (!f) {
        throw std::runtime_error("Failed to load " + confDir.string() + "/config.json");
    }

    nlohmann::json j;
    f >> j;
    j.get_to(cfg);

    spdlog::debug("Loaded config from {}/config.json", confDir.string());

    bootstrap();
}

void Server::bootstrap() {
#ifdef JADE_DEBUG
    app.loglevel(crow::LogLevel::DEBUG);
#endif

    crow::mustache::set_global_base("www");
    crow::mustache::set_base("www");
    assetBaseDir = "www";

    initEndpoints();

    app.add_static_dir();
    app
        .server_name("LunarWatcher/Jade")
        .multithreaded()
        .use_compression(crow::compression::GZIP)
        .port(cfg.port);
}

void Server::initEndpoints() {
    WebProvider::init(this);
}

void Server::run() {
    app.run();
}

}
