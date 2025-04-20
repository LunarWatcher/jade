#include "jade/core/Server.hpp"
#include <filesystem>

int main() {
    std::filesystem::path confDir;
#ifdef JADE_DEBUG
    confDir = "./";
#else
    confDir = "/etc/jade";
#endif

    jade::Server server(confDir);

    server.run();
}
