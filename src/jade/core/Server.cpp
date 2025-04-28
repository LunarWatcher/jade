#include "Server.hpp"
#include "crow/app.h"
#include "jade/api/APIProvider.hpp"
#include "jade/db/ConnectionPool.hpp"
#include "jade/db/Migration.hpp"
#include "jade/web/WebProvider.hpp"

#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

namespace jade {

Server::Server(const std::filesystem::path& confDir) {
    inst = this;
    std::ifstream f(confDir / "config.json");

    if (!f) {
        throw std::runtime_error("Failed to load " + confDir.string() + "/config.json");
    }

    nlohmann::json j;
    f >> j;
    j.get_to(cfg);

    spdlog::debug("Loaded config from {}/config.json", confDir.string());

    pool = std::make_shared<ConnectionPool>(
        cfg.getConnString()
    );

    bootstrap();
    dbMigrations();

}

Server::~Server() {
    inst = nullptr;
}

void Server::bootstrap() {
#ifdef JADE_DEBUG
    app.loglevel(crow::LogLevel::DEBUG);
#endif

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
    m.pushVersion(R"(
    CREATE TABLE Jade.Users (
        UserID      SERIAL          PRIMARY KEY,
        Username    TEXT UNIQUE     NOT NULL,
        Password    TEXT            NOT NULL,
        Salt        TEXT            NOT NULL,
        Version     INTEGER         NOT NULL,
        IsAdmin     BOOLEAN         NOT NULL
    );
    CREATE TABLE Jade.Libraries (
        LibraryID   SERIAL          PRIMARY KEY,
        Location    TEXT
    );
    CREATE TABLE Jade.Books (
        BookID      SERIAL          PRIMARY KEY,
        LibraryID   INTEGER         REFERENCES Libraries(LibraryID),
        Title       TEXT            NOT NULL,
        Description TEXT,
        AgeRating   INTEGER
    );
    )");

    pool->acquire([&](auto& conn) {
        {
            pqxx::work tx{conn};
            tx.exec("CREATE SCHEMA IF NOT EXISTS Jade");
            tx.commit();
        }
        m.prepMetatables(conn);
        m.exec(conn);
    });
}

void Server::run() {
    app.run();
}

}
