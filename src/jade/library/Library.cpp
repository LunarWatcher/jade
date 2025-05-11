#include "Library.hpp"
#include "jade/health/HealthCheck.hpp"
#include "spdlog/spdlog.h"
#include <iterator>

#include <jade/core/Server.hpp>

namespace jade {

Library::Library(pqxx::connection& conn) {
    pqxx::work w(conn);
    auto libraries = w.query<int64_t, std::string>("SELECT LibraryID, Location FROM Jade.Libraries");

    std::transform(
        libraries.begin(), libraries.end(), 
        std::inserter(sources, sources.begin()),
        [](const auto& row) -> std::pair<int64_t, Source> {
            return {
                std::get<0>(row),
                Source {
                    std::get<1>(row)
                }
            };
        }
    );

    if (sources.size() == 0) {
        spdlog::warn("No active libraries");
    }

}

void Library::scan() {
    while (true) {
        for (auto& [id, source] : sources) {
            // TODO
        }

        if (sources.size() == 0) {
            spdlog::info("Scanner: No libraries exist to scan");
        }
        std::this_thread::sleep_for(std::chrono::minutes(15));
    }
}

void Library::initScanProcess() {
    runner = std::thread(
        std::bind(&Library::scan, this)
    );
}

bool Library::createLibrary(Server* serv, const std::string& location) {
    return serv->pool->acquire<bool>([&](auto& conn) {
        pqxx::work w(conn);

        auto exists = w.query1<bool>(
            "SELECT EXISTS(SELECT 1 FROM Jade.Libraries WHERE Location = $1)", 
            pqxx::params {location}
        );

        if (std::get<0>(exists)) {
            return false;
        }

        auto res = w.exec(R"(INSERT INTO Jade.Libraries (LibraryID, Location) VALUES (DEFAULT, $1) RETURNING LibraryID)",
            pqxx::params {
                location
            }
        ).one_row();
        auto id = res.at(0).as<int64_t>();
        spdlog::info("LibraryID {} (loc: {}) created", id, location);

        w.commit();
        std::unique_lock l(m);
        sources[id] = Source {
            location
        };
        return true;
    });
}

std::vector<health::HealthResult> Library::checkHealth() {
    std::vector<health::HealthResult> out;

    {
        std::vector<health::HealthCheck> sourceChecks;

        for (auto& [id, source] : this->sources) {
            auto isValid = source.isValid();
            sourceChecks.push_back({
                isValid,
                source.dir.string(),
                isValid ? "" : "Path does not exist, or is not a directory"
            });
        }

        out.push_back({
            "Library sources",
            sourceChecks
        });
    }

    return out;
}

}
