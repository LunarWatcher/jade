#pragma once

#include "HealthCheck.hpp"
#include "crow/json.h"
#include <memory>

namespace jade::health {

class HealthCore {
private:
    std::vector<std::shared_ptr<HealthCheckable>> checks; 

public:

    std::vector<HealthResult> getResults() {
        std::vector<HealthResult> results;
        for (auto& check : checks) {
            auto intermediates = check->checkHealth();
            results.insert(results.end(), intermediates.begin(), intermediates.end());
        }

        return results;
    }

    crow::json::wvalue getJSONResults() {
        std::vector<crow::json::wvalue> out;

        auto results = getResults();
        int failCount = 0;
        int totalChecks = 0;

        for (auto& result : results) {
            std::vector<crow::json::wvalue> checks;

            for (auto& check : result.checks) {
                failCount += (int) !check.passed;
                totalChecks += 1;
                checks.push_back({
                    {"Passed", check.passed},
                    {"Identifier", check.checkIdentifier},
                    {"Message", check.message}
                });
            }

            out.push_back({
                {"NodeIdentifier", result.nodeIdentifier},
                {"Checks", checks}
            });
        }

        crow::json::wvalue res = {
            {"Results", out},
            {"TotalChecks", totalChecks},
            {"Fails", failCount}
        };
        // Hack for 0 not being falsy in mustache
        if (failCount > 0) {
            res["Fails"] = failCount;
        } else {
            res["Fails"] = false;
        }
        return res;
    }

    void registerChecks(const std::vector<std::shared_ptr<HealthCheckable>>& checks) {
        this->checks.insert(this->checks.end(), checks.begin(), checks.end());
    }

};

}
