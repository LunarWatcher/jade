#pragma once

#include <filesystem>
#include <fstream>
#include <rfl/DefaultIfMissing.hpp>
#include <spdlog/spdlog.h>
#include <string>
#include <rfl.hpp>
#include <rfl/json.hpp>

namespace jade {

/**
 * Root class for the config. Also corresponds exactly to the `config.json` file.
 */
struct ServerConfig {
    /**
     * The port the webserver runs at. Should differ from :80/:443 if using nginx or similar
     */
    uint16_t port = 6969;

    /**
     * The host the database is at. This must NOT include the port!
     * The port is currently not configurable, and assumes 5432
     * points to the correct postgres installation.
     */
    std::string dbHost = "localhost";

    std::string dbPassword;

    /**
     * The database name to connect to. This should usually not be modified outside
     * tests. The default name generated by the install script is `jade`, the default set
     * here.
     */
    std::string dbName = "jade";

    /**
     * The postgres user to connect under. Just like the database name, the default name
     * generated by the install script is `jade`.
     */
    std::string dbUsername = "jade";

    std::filesystem::path thumbnailCacheDir = "./thumbnails";

    std::string getConnString();
    void createDirectories() {
        if (!std::filesystem::exists(thumbnailCacheDir)) {
            std::filesystem::create_directories(thumbnailCacheDir);
        }
    }

};

static void loadConfig(ServerConfig& cfg, const std::filesystem::path& confDir) {
    std::ifstream f(confDir / "config.json");

    if (!f) {
        throw std::runtime_error("Failed to load " + confDir.string() + "/config.json");
    }

    auto res = rfl::json::read<ServerConfig, rfl::DefaultIfMissing>(f);
    if (!res) {
        spdlog::error("Failed to load config file: {}", res.error().what());
        throw std::runtime_error("Invalid config");
    }

    cfg = res.value();
    spdlog::debug("Loaded config from {}/config.json", confDir.string());
}

}
