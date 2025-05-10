#include "SettingsRequests.hpp"

void jade::from_json(const nlohmann::json& j, jade::CreateLibRequest& o) {
    j.at("location").get_to(o.location);
}
