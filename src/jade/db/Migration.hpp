#pragma once

#include <vector>
#include <string>
#include <spdlog/spdlog.h>

#include <pqxx/pqxx>

namespace jade {

class Migration {
private:
    std::vector<std::string> queries;
    static inline std::string IDENTIFIER = "__jade_migration_version__";

public:
    
    Migration& pushVersion(const std::string& query);
    void exec(pqxx::connection& conn);

    static void prepMetatables(pqxx::connection& conn) {
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
