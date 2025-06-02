#include "CLIMigrations.hpp"
#include "jade/conf/ServerConfig.hpp"
#include "jade/db/ConnectionPool.hpp"
#include "jade/db/migrations/00Order.hpp"
#include <iostream>

namespace jade::CLI {


CLIMigrations::CLIMigrations(const std::filesystem::path& confDir) {
    loadConfig(cfg, confDir);
    pool = std::make_shared<ConnectionPool>(cfg.getConnString());
}

void CLIMigrations::up() {
    pool->acquire<void>([&](auto& conn) {
        m.prepMetatables(conn);
        m.upgrade(conn);
    });
}

void CLIMigrations::down(int64_t to) {

    pool->acquire<void>([&](auto& conn) {
        m.prepMetatables(conn);
        m.downgrade(conn, to);
    });
}

void CLIMigrations::get() {
    pool->acquire<void>([&](auto& conn) {
        m.prepMetatables(conn);
        pqxx::work w(conn);
        auto migrations = getMigrations();
        auto currMigration = m.getCurrentVersion(w);

        if (currMigration > migrations->size()) {
            spdlog::error(
                "Your database is currently on migration version {}, but only {} were found in code. "
                "Did you accidentally switch between branches and forget to downgrade before? "
                "This situation cannot be repaired without a rollback of Jade. Be more careful next time :)",
                currMigration, migrations->size()
            );
            exit(0);
        }

        std::cout << "Current version is " << currMigration;
        if (currMigration != 0) {
            std::cout 
                << ": " 
                << migrations->at(currMigration - 1)->name();
        } 
        std::cout << "\n\nReference table for CLI downgrades:\n";
        std::cout 
            << std::setw(8) << std::left << "Version"
            << std::setw(10) << std::left << "Relative"
            << std::setw(32) << std::left << "Name"
            << std::endl;
        for (int i = 0; i < migrations->size(); ++i) {
            std::cout << std::setw(8) << std::left;
            if (i < currMigration) {
                std::cout << i + 1;
            } else {
                std::cout << "---";
            }

            std::cout << std::setw(10) << std::left;
            if (i < currMigration) {
                std::cout << i - currMigration;
            } else {
                std::cout << "---";
            }

            std::cout << std::setw(32) << migrations->at(i)->name() << std::left;
            std::cout << std::endl;
        }
    });
}

}
