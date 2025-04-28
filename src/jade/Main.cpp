#include "jade/core/Server.hpp"
#include <filesystem>

#include "spdlog/cfg/env.h"

int main() {
    std::filesystem::path confDir;
#ifdef JADE_DEBUG
    confDir = "./";
#else
    confDir = "/etc/jade";
#endif

    spdlog::cfg::load_env_levels();

    jade::Server server(confDir);

    server.run();
}
