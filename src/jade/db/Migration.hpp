#pragma once

#include <string>
#include <spdlog/spdlog.h>

#include <pqxx/pqxx>

namespace jade {

class Migration {
private:
    static inline std::string IDENTIFIER = "__jade_migration_version__";

public:
    void upgrade(pqxx::connection& conn);
    void downgrade(pqxx::connection& conn, int64_t downgradeTo = 0);

    int64_t getCurrentVersion(pqxx::work& tx);

    static void prepMetatables(pqxx::connection& conn) {
        {
            pqxx::work tx{conn};
            tx.exec("CREATE SCHEMA IF NOT EXISTS Jade");
            tx.commit();
        }
        spdlog::info("Initialising migration table...");
        pqxx::work tx{conn};
        tx.exec(R"(
        CREATE TABLE IF NOT EXISTS Jade.Migration (
            Name TEXT PRIMARY KEY,
            Version INTEGER NOT NULL
        );
        )");
        tx.commit();
    }

};

}
