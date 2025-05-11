#pragma once

#include <string>
#include <vector>

namespace jade::health {

struct HealthCheck {
    bool passed;
    std::string checkIdentifier;
    std::string message;
};

struct HealthResult {
    std::string nodeIdentifier;
    std::vector<HealthCheck> checks;
};

struct HealthCheckable {
    virtual std::vector<HealthResult> checkHealth() = 0;
};

}
