#include "jade/core/Server.hpp"
#include <filesystem>
#include <CLI/CLI.hpp>

#include "spdlog/cfg/env.h"

int main(int argc, char** argv) {
    std::filesystem::path confDir;
#ifdef JADE_DEBUG
    confDir = "./";
#else
    confDir = "/etc/jade";
#endif

    spdlog::cfg::load_env_levels();

    CLI::App app{"Ebook server", "jade"};
    argv = app.ensure_utf8(argv);

    jade::Flags conf;
    app.add_flag("-d,--debug-crow", conf.debugCrow)
        ->default_val(false)
        ->description("Whether or not to enable verbose crow logging. This does NOT affect the log level "
                      "of Jade itself, which uses a separate logging system.");

    app.add_option("-o,--override-config", confDir)
        ->default_val(confDir)
        ->required(false)
        ->description("Optional override location for the config. "
                      "This should only be used for tests, or if you're really, really "
                      "sure what you're doing.");

    app.add_option("--purge", conf.purgeDatabase)
        ->default_val(false)
        ->required(false)
        ->description("Whether or not to purge the database after load. "
                      "Only takes effect if the database name is 'jadetest', "
                      "generates a warning otherwise");

    CLI11_PARSE(app, argc, argv);

    jade::Server server(confDir, conf);

    server.run();
}
