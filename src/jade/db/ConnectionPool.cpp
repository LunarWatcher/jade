#pragma once

#include "jade/db/ConnectionPool.hpp"

namespace jade {

ConnectionPool::ConnectionPool(
    const std::string& connectionString
) : connectionString(connectionString) {
}

void ConnectionPool::acquire(std::function<void(pqxx::connection&)> callback) {
    // TODO: make a real connection pool
    auto conn = std::make_shared<pqxx::connection>(
        connectionString
    );
    callback(
        *conn
    );

}

}
