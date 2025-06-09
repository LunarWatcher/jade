#include "Server.hpp"
#include "crow/app.h"
#include "jade/api/APIProvider.hpp"
#include "jade/api/books/BookAPI.hpp"
#include "jade/core/Typedefs.hpp"
#include "jade/db/ConnectionPool.hpp"
#include "jade/db/Migration.hpp"
#include "jade/library/Library.hpp"
#include "jade/web/WebProvider.hpp"

#include <filesystem>
#include <spdlog/spdlog.h>

namespace jade {

Server::Server(const std::filesystem::path& confDir, Flags runtimeConfig) : app(), runtimeConfig(runtimeConfig) {
    inst = this;

    loadConfig(cfg, confDir);
    cfg.createDirectories();

    pool = std::make_shared<ConnectionPool>(
        cfg.getConnString()
    );

    bootstrap();
    dbMigrations();

    if (this->cfg.dbName == "jadetest" && runtimeConfig.purgeDatabase) {
        pool->acquire<void>([](auto& conn) {
            pqxx::work w(conn);
            w.exec("DROP SCHEMA Jade CASCADE;");
            w.commit();
        });
    } else if (runtimeConfig.purgeDatabase) {
        spdlog::warn("--purge was specified, but the database name is not jadetest. "
                     "You've been protected from yourself, no purge has been performed. "
                     "If you really want to purge prod, you can use scripts/dev/purge.sh, "
                     "or copy the commands out of it and deal with it in whatever way you see "
                     "fit. Purging prod with --purge is not supported");
    }

    pool->acquire<void>([this](auto& conn) {
        lib = std::make_shared<Library>(
            this,
            conn
        );
        lib->initScanProcess();
    });

    initHealth();

}

Server::~Server() {
    inst = nullptr;
}

void Server::bootstrap() {
    if (runtimeConfig.debugCrow) {
        app.loglevel(crow::LogLevel::DEBUG);
    } else {
        // TODO: make this configurable with warning as a default
        app.loglevel(crow::LogLevel::WARNING);
    }

    crow::mustache::set_global_base("www");
    crow::mustache::set_base("www");
    assetBaseDir = "www";

    app.add_static_dir();

    initEndpoints();

    app
        .server_name("LunarWatcher/Jade")
        .multithreaded()
        .use_compression(crow::compression::GZIP)
        .port(cfg.port);
}

void Server::initEndpoints() {
    WebProvider::init(this);
    spdlog::debug("Web endpoints registered");
    APIProvider::init(this);
    spdlog::debug("API endpoints registered");

}

void Server::dbMigrations() {
    Migration m;
    // TODO: Figure out how to disambiguate authors with the same name (additional author data?)

    pool->acquire<void>([&](auto& conn) {
        m.prepMetatables(conn);
        m.upgrade(conn);
    });
}

void Server::initHealth() {
    healthCore.registerChecks({
        lib,
        pool
    });
}

void Server::run() {
    app.run();
}

void Server::kill() {
    lib->kill();
    app.stop();
}

}
