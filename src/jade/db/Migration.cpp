#include "Migration.hpp"

namespace jade {

Migration& Migration::pushVersion(const std::string& query) {
    queries.push_back(query);

    return *this;
}

void Migration::exec(pqxx::connection& conn) {
    pqxx::work work(conn);
    spdlog::debug("Preparing migration...");

    auto res = work.exec(
        "SELECT Version FROM Jade.Migration WHERE Name = $1",
        pqxx::params { IDENTIFIER }
    );
    
    long long currVersion = 0;
    if (!res.empty()) {
        // If there is a result, ensure it's a long long. 
        //currVersion = res.at(0).at(0).get<long long>().value_or(-1);
        currVersion = res.at(0).at(0).get<long long>().value_or(-1);
        // If it isn't, the database has been manually fucked with. Throw and let the host struggle (or future me if I've done a dumdum somewhere)
        if (currVersion <= -1) {
            throw std::runtime_error("Panic (programmer error/host error): invalid version stored in database. Wipe database");
        }
    }
    if ((size_t) currVersion == queries.size()) {
        spdlog::info("Migrations up to date (v{})", currVersion);
        return;
    }

    for (size_t i = currVersion; i < queries.size(); ++i) {
        spdlog::info("{} to v{}", i == 0 ? "Initialising" : "Updating", i + 1);
        work.exec(queries.at(i));
    }

    work.exec(
        "INSERT INTO Jade.Migration (Name, Version) VALUES ($1, $2) ON CONFLICT (Name) DO UPDATE SET Version = $2;",
        pqxx::params { IDENTIFIER, this->queries.size() }
    );

    work.commit();
}

}
