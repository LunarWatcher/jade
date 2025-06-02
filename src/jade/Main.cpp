#include "jade/cli/CLIMigrations.hpp"
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
    app.require_subcommand(1, 1);

    auto run = app.add_subcommand("run");

    jade::Flags conf;
    run->add_flag("-d,--debug-crow", conf.debugCrow)
        ->default_val(false)
        ->description("Whether or not to enable verbose crow logging. This does NOT affect the log level "
                      "of Jade itself, which uses a separate logging system.");

    run->add_option("-o,--override-config", confDir)
        ->default_val(confDir)
        ->required(false)
        ->description("Optional override location for the config. "
                      "This should only be used for tests, or if you're really, really "
                      "sure what you're doing.");

    run->add_option("--purge", conf.purgeDatabase)
        ->default_val(false)
        ->required(false)
        ->description("Whether or not to purge the database after load. "
                      "Only takes effect if the database name is 'jadetest', "
                      "generates a warning otherwise");


    auto migrate = app.add_subcommand("migrate")
        ->require_subcommand(1, 1)
        ->description(
            "Manually run migrations. You usually don't want to do this unless "
            "you're shifting between branches, or downgrading Jade. Upgrades are automatically "
            "run on boot."
        );
    auto up = migrate->add_subcommand("up")
        ->description("Upgrades to the latest version. Does nothing if up to date");
    auto down = migrate->add_subcommand("down");
    int64_t downgradeTo = 0;
    down->add_option("downgrade-to", downgradeTo)
        ->default_val(0)
        ->description(
            "Which version to downgrade to. If left unspecified or set to 0, all versions are downgraded. "
            "If set to a positive number, that version is reverted to. Negative versions are relative to the current "
            "migration number. "
        );

    auto migrationVersion = migrate->add_subcommand("get")
        ->description("Prints the name of the latest version");

    run->parse_complete_callback([&] {
        jade::Server server(confDir, conf);
        server.run();
    });
    up->parse_complete_callback([&] {
        jade::CLI::CLIMigrations m(confDir);
        m.up();
    });
    down->parse_complete_callback([&] {
        jade::CLI::CLIMigrations m(confDir);
        m.down(downgradeTo);
    });
    migrationVersion->parse_complete_callback([&] {
        jade::CLI::CLIMigrations m(confDir);
        m.get();
    });
    CLI11_PARSE(app, argc, argv);


}
