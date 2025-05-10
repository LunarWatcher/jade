#include "jade/db/ConnectionPool.hpp"

namespace jade {

ConnectionPool::ConnectionPool(
    const std::string& connectionString
) : connectionString(connectionString) {
}


}
