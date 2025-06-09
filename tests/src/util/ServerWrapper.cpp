#include "ServerWrapper.hpp"
#include "Conf.hpp"
#include "jade/conf/ServerConfig.hpp"
#include "jade/core/Server.hpp"
#include "jade/db/ConnectionPool.hpp"
#include "jade/db/migrations/01Initial.hpp"
#include <exception>
#include <filesystem>
#include <fstream>
#include <stdexcept>

namespace jade::tests {

ServerWrapper::ServerWrapper(bool setupAdminUser) {
    if (!std::filesystem::exists(CONFIG_DIR)) {
        throw std::runtime_error("Please don't fuck around with __test_.*__ dirs while tests are running.");
    }
    
    clearThumbnails();
    wipeDatabase();

    serv = std::make_shared<Server>(
        CONFIG_DIR,
        Flags{}
    );

    runner = std::thread([this]() {
        this->serv->run();
    });

    if (setupAdminUser) {
        auto [u, p] = getAdminCreds();
        createUser(u, p, true);
    }
    // Seems to be required to force enough time for the server to start.
    // This feels like it could be flaky 
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

ServerWrapper::~ServerWrapper() {
    spdlog::debug("Killing server");
    serv->kill();
    // serv->kill() invokes app.stop() or whatever, so because that's all this thread does, it should just terminate
    // after this.
    runner.join();
}

void ServerWrapper::createUser(const std::string& username, const std::string& unhashedPassword, bool isAdmin) {
    spdlog::debug("Creating user {}/{}", username, unhashedPassword);
    
    serv->pool->acquire<void>([&](auto& conn) {
        pqxx::work tx(conn);
        
        auto password = Hash::hash(unhashedPassword, std::nullopt);
        tx.exec(R"(
        INSERT INTO Jade.Users (Username, Password, Salt, Version, IsAdmin)
        VALUES ($1, $2, $3, $4, $5);
        )", pqxx::params {
            username,
            password.hash,
            password.salt,
            password.version,
            isAdmin
        });
        tx.commit();
    });
}

void ServerWrapper::wipeDatabase() {
    ServerConfig cfg;
    loadConfig(cfg, CONFIG_DIR);
    pqxx::connection conn(cfg.getConnString());
    try {
        pqxx::work tx(conn);
        MInitial{}.downgrade(tx);
        tx.commit();
    } catch (const std::exception& e) {
        // The schema not being found is fine
        if (std::string(e.what()).find("schema \"jade\" does not exist") == std::string::npos) {
            throw;
        }
    }
}

void ServerWrapper::clearThumbnails() {
    if (std::filesystem::exists(THUMBNAIL_DIR)) {
        std::filesystem::remove_all(THUMBNAIL_DIR);
    }
}

void ServerWrapper::writeConfigFile() {
    ServerConfig cfg {
        .port = 56432,
        .dbPassword = TestConf::PSQL_PASSWORD,
        .dbName = "jadetest",
        .thumbnailCacheDir = THUMBNAIL_DIR
    };

    if (cfg.dbPassword.size() == 0) {
        throw std::runtime_error("global PSQL_PASSWORD has failed");
    }

    std::filesystem::create_directories(CONFIG_DIR);
    std::filesystem::create_directories(THUMBNAIL_DIR);
    std::ofstream f(CONFIG_FILE);
    if (!f) {
        throw std::runtime_error("Failed to open config file");
    }
    rfl::json::write(cfg, f);

}

}
