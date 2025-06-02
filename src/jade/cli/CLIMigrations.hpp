#pragma once

#include "jade/conf/ServerConfig.hpp"
#include "jade/db/ConnectionPool.hpp"
#include "jade/db/Migration.hpp"
#include <cstdint>
#include <memory>

namespace jade::CLI {

class CLIMigrations {
private:
    ServerConfig cfg;
    std::shared_ptr<ConnectionPool> pool;
    Migration m;

public:
    CLIMigrations(const std::filesystem::path& confDir);

    void up();
    void down(int64_t to);
    void get();
};


}
