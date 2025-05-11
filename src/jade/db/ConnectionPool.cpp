#include "jade/db/ConnectionPool.hpp"

namespace jade {

ConnectionPool::ConnectionPool(
    const std::string& connectionString
) : connectionString(connectionString) {
}

std::vector<health::HealthResult> ConnectionPool::checkHealth() {
    std::vector<health::HealthResult> out;

    {
        std::vector<health::HealthCheck> checks;

        // TODO: This is shit, but this is the best I can do until I implement an actual pool
        acquire<void>([&](pqxx::connection& conn) {
            checks.push_back({
                conn.is_open(),
                "Is the database reachable?",
                conn.is_open() ? "" : "Database not reachable"
            });
        });

        out.push_back({
            "Database connection pool",
            checks
        });
    }

    return out;
}

}
