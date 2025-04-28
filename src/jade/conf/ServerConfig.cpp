#include "ServerConfig.hpp"

namespace jade {

std::string ServerConfig::getConnString() {
    std::stringstream ss;
    ss << "host=" << dbHost << " ";
    ss << "password=" << dbPassword << " ";
    ss << "user=" << dbUsername << " ";
    ss << "dbname=" << dbName;

    return ss.str();
}

}
