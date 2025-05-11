#pragma once

#include "jade/health/HealthCheck.hpp"
#include <string>
#include <pqxx/pqxx>

namespace jade {

class ConnectionPool : public health::HealthCheckable {
private:
    const std::string connectionString;
public:
    ConnectionPool(
        const std::string& connectionString
    );

    template <typename T>
    auto acquire(std::function<T(pqxx::connection&)> callback) -> T {
        auto conn = std::make_shared<pqxx::connection>(connectionString);
        if constexpr (std::is_same_v<T, void>) {
            callback(*conn);
        } else {
            return callback(*conn);
        }
    }

    std::vector<health::HealthResult> checkHealth() override;

};

}
