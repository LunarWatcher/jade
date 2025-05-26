#pragma once

#include "spdlog/spdlog.h"
#include <stdexcept>
#include <string>

namespace TestConf {

inline std::string PSQL_PASSWORD = "";

struct Load {
    bool hasPSQL_PASSWORD = false;

    void operator()(const std::string_view& name, const std::string_view& val) {
        if (name == "PSQL_PASSWORD") {
            PSQL_PASSWORD = std::string(val);
        } else {
            spdlog::warn("Found unknown key '{}'.", name);
        }
    }

    void finish() {
        if (hasPSQL_PASSWORD == false) {
            throw std::runtime_error("Missing required variable: PSQL_PASSWORD");
        }
    }
};

}

