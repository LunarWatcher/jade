#include "Server.hpp"
#include "crow/app.h"
#include "crow/middlewares/session.h"
#include "jade/api/APIProvider.hpp"
#include "jade/api/books/BookAPI.hpp"
#include "jade/core/Typedefs.hpp"
#include "jade/db/ConnectionPool.hpp"
#include "jade/db/Migration.hpp"
#include "jade/library/Library.hpp"
#include "jade/web/WebProvider.hpp"

#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

namespace jade {

Server::Server(const std::filesystem::path& confDir, Flags runtimeConfig) : app(), runtimeConfig(runtimeConfig) {
    inst = this;
    std::ifstream f(confDir / "config.json");

    if (!f) {
        throw std::runtime_error("Failed to load " + confDir.string() + "/config.json");
    }

    nlohmann::json j;
    f >> j;
    j.get_to(cfg);
    cfg.createDirectories();

    spdlog::debug("Loaded config from {}/config.json", confDir.string());

    pool = std::make_shared<ConnectionPool>(
        cfg.getConnString()
    );


    bootstrap();
    dbMigrations();

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
    m.pushVersion(R"(
    -- Object tables {{{
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
        Location    TEXT            NOT NULL,
        AgeRating   INTEGER
    );
    CREATE TABLE Jade.Tags (
        TagID       SERIAL          PRIMARY KEY,
        TagName     TEXT            NOT NULL
    );
    CREATE TABLE Jade.Collections (
        CollectionID            SERIAL  PRIMARY KEY,
        CollectionName          TEXT    NOT NULL,
        CollectionDescription   TEXT
    );
    CREATE TABLE Jade.Series (
        SeriesID            SERIAL  PRIMARY KEY,
        SeriesName          TEXT    NOT NULL,
        SeriesDescription   TEXT
    );
    CREATE TABLE Jade.Books (
        BookID      SERIAL          PRIMARY KEY,
        FileName    TEXT            NOT NULL,
        LibraryID   INTEGER         REFERENCES Libraries(LibraryID) ON DELETE CASCADE,
        SeriesID    INTEGER         REFERENCES Series(SeriesID) ON DELETE CASCADE,
        Title       TEXT            NOT NULL,
        ISBN        TEXT,
        Description TEXT,
        AgeRating   INTEGER
    );
    -- }}}
    -- Relational tables {{{
    CREATE TABLE Jade.BookTags (
        BookID INTEGER NOT NULL REFERENCES Jade.Books(BookID) ON DELETE CASCADE,
        TagID  INTEGER NOT NULL REFERENCES Jade.Tags(TagID) ON DELETE CASCADE,
        PRIMARY KEY (BookID, TagID)
    );
    CREATE TABLE Jade.BookSeries (
        BookID INTEGER NOT NULL REFERENCES Jade.Books(BookID) ON DELETE CASCADE,
        SeriesID  INTEGER NOT NULL REFERENCES Jade.Series(SeriesID) ON DELETE CASCADE,
        BookIndex       INTEGER NOT NULL,
        PRIMARY KEY (BookID, SeriesID)
    );
    -- }}}
    )");

    pool->acquire<void>([&](auto& conn) {
        {
            pqxx::work tx{conn};
            tx.exec("CREATE SCHEMA IF NOT EXISTS Jade");
            tx.commit();
        }
        m.prepMetatables(conn);
        m.exec(conn);
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

}
