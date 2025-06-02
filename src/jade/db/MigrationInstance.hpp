#pragma once

#include "jade/db/ConnectionPool.hpp"
#include <string>

namespace jade {

struct MigrationInstance {
    virtual ~MigrationInstance() = default;
    virtual void upgrade(pqxx::work& tx) = 0;
    /**
     * Downgrades a migration. Primarily used in manual environments 
     */
    virtual void downgrade(pqxx::work& tx) = 0;

    /**
     * Describes a name for the migration used exclusively for logging. 
     * Migrations are automatically described by an index, so this is purely for 
     * providing context to logging.
     */
    virtual std::string name() = 0;
};

}
