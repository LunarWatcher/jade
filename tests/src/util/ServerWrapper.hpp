#pragma once

#include "jade/conf/ServerConfig.hpp"
#include "jade/core/Server.hpp"
#include <cpr/cprtypes.h>
namespace jade::tests {

class ServerWrapper {
public:
    static inline const std::filesystem::path CONFIG_DIR = "./__test_conf_dir__/";
    static inline const std::filesystem::path CONFIG_FILE = CONFIG_DIR / "config.json";
    static inline const std::filesystem::path THUMBNAIL_DIR = "./__test_thumbnail_dir__/";

    std::shared_ptr<Server> serv;
    std::thread runner;

    ServerWrapper(
        bool setupAdminUser = true
    );
    ~ServerWrapper();

    /**
     * Uses a direct database edit to create a user.
     */
    void createUser(
        const std::string& username,
        const std::string& password,
        bool isAdmin
    );
    /**
     * Returns credentials for a single admin user. This user is created by default, but must be created manually if
     * createAdminUser is set to false when creating the wrapper.
     */
    std::pair<std::string, std::string> getAdminCreds() {
        return {"admin", "password"};
    }
    /**
     * Returns credentials for a single user. Note that this user must be manually created before using the credentials,
     * as this isn't done automatically.
     */
    std::pair<std::string, std::string> getNonAdminCreds() {
        return {"user", "password"};
    }
    void clearThumbnails();
    void wipeDatabase();
    std::string getUrl() {
        return "http://localhost:56432";
    }

    static void writeConfigFile();

    cpr::Url operator/(const std::string_view& frag) {
        return getUrl().append("/").append(frag);
    } 
};

}
