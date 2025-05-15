#pragma once

#include "crow/json.h"
#include <vector>
namespace jade::Util {

template <typename T>
std::vector<crow::json::wvalue> vec2json(const std::vector<T>& src) {
    std::vector<crow::json::wvalue> out;

    std::transform(src.begin(), src.end(), std::back_inserter(out), [](const auto& o) {
        return o.toJson();
    });

    return out;
}

}
