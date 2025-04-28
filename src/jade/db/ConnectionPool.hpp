#pragma once

#include <string>
#include <pqxx/pqxx>

namespace jade {

class ConnectionPool {
private:
    const std::string connectionString;
public:
    ConnectionPool(
        const std::string& connectionString
    );

    void acquire(std::function<void(pqxx::connection& conn)>);

};

}
