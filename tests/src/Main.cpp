#include "Conf.hpp"
#include "spdlog/spdlog.h"
#include "stc/StringUtil.hpp"
#include <fstream>
#include <stdexcept>
#include <string>

#define CATCH_CONFIG_RUNNER
#include "catch2/catch_session.hpp"

int main(int argc, const char* argv[]) {

    spdlog::info("Loading .env...");

    std::ifstream f("../.env");

    if (!f) {
        f.clear();
        f.open("./.env");

        if (!f) {
            spdlog::error("Panic: failed to locate .env. Tried: ./.env, ../.env");
            spdlog::error("This is not a test error; this is a you error. Read the README and try again");

            throw std::runtime_error("Failed to load .env");
        }
    }

    std::string buff;
    TestConf::Load l;

    while (std::getline(f, buff)) {
        if (buff.starts_with("#")) {
            continue;
        }

        auto lineVal = stc::string::split(buff, '=', 1);
        if (lineVal.size() == 0) {
            continue;
        } else if (lineVal.size() == 1) {
            spdlog::error("Skipping invalid line: \"{}\"", buff);
            continue;
        }
        l(lineVal.at(0), lineVal.at(1));
    }

    spdlog::info(".env loaded successfully.");

    return Catch::Session().run(argc, argv);
}
