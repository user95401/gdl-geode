#pragma once
#include <string>
#include <json/json.hpp>

using namespace nlohmann;

namespace gdlutils {
    json loadJson(const std::string& name);
}