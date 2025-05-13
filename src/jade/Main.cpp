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
    app.add_flag("--debug-crow", conf.debugCrow)
        ->default_val(false)
        ->description("Whether or not to enable verbose crow logging. This does NOT affect the log level "
                      "of Jade itself, which uses a separate logging system.");

    CLI11_PARSE(app, argc, argv);

    jade::Server server(confDir, conf);

    server.run();
}
