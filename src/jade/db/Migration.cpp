#include "Migration.hpp"
#include "jade/db/migrations/00Order.hpp"

namespace jade {

int64_t Migration::getCurrentVersion(pqxx::work& tx) {
    auto res = tx.exec(
        "SELECT Version FROM Jade.Migration WHERE Name = $1",
        pqxx::params { IDENTIFIER }
    );
    
    int64_t currVersion = 0;
    if (!res.empty()) {
        // If there is a result, ensure it's a long long. 
        //currVersion = res.at(0).at(0).get<long long>().value_or(-1);
        currVersion = res.at(0).at(0).get<long long>().value_or(-1);
        // If it isn't, the database has been manually fucked with. Throw and let the host struggle (or future me if I've done a dumdum somewhere)
        if (currVersion <= -1) {
            throw std::runtime_error("Panic (programmer error/host error): invalid version stored in database. Wipe database");
        }
    }
    return currVersion;
}

void Migration::upgrade(pqxx::connection& conn) {
    pqxx::work work(conn);
    spdlog::debug("Preparing migration...");

    auto currVersion = getCurrentVersion(work);
    auto queries = getMigrations();
    if ((size_t) currVersion == queries->size()) {
        spdlog::info("Migrations up to date (latest is v{}; {})", currVersion, queries->back()->name());
        return;
    }

    for (size_t i = currVersion; i < queries->size(); ++i) {
        auto& query = queries->at(i);
        spdlog::info("Upgrading to {} (v{})", query->name(), i + 1);
        query->upgrade(work);
    }

    work.exec(
        "INSERT INTO Jade.Migration (Name, Version) VALUES ($1, $2) ON CONFLICT (Name) DO UPDATE SET Version = $2;",
        pqxx::params {
            IDENTIFIER,
            queries->size() 
        }
    );

    work.commit();
}

void Migration::downgrade(pqxx::connection& conn, int64_t downgradeTo) {
    pqxx::work tx(conn);

    auto currVersion = getCurrentVersion(tx);
    if (currVersion == 0) {
        spdlog::error("Downgrade failed: no versions to downgrade");
        throw std::runtime_error("Nothing to drop");
    }
    auto targetVersion = downgradeTo < 0 ? currVersion - downgradeTo : (downgradeTo > 0 ? downgradeTo - 1 : 0);
    if (targetVersion < 0) {
        spdlog::error(
            "Cannot roll back {} revisions; there are only {} installed revisions",
            downgradeTo, currVersion
        );
        throw std::runtime_error("Bad target version");
    }

    auto migrations = getMigrations();
    for (int64_t i = currVersion - 1; i >= targetVersion; --i) {
        migrations->at(i)->downgrade(tx);
        spdlog::info("Downgraded v{} ({})", i + 1, migrations->at(i)->name());
    }

    tx.commit();
}

}
