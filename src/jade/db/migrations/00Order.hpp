#pragma once

#include "jade/db/MigrationInstance.hpp"
#include "01Initial.hpp"

#include <vector>

namespace jade {

struct Migrations {
    std::vector<MigrationInstance*> instances;

    ~Migrations() {
        for (auto& instance : instances) {
            delete instance;
        }
    }

    decltype(instances)* operator->() { return &instances; }
};

inline Migrations getMigrations() {
    return {{
        new MInitial,
    }};
}

}
