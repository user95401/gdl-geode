#pragma once
#include <string>
#include <json/json.hpp>
#include <Geode/Geode.hpp>

using namespace nlohmann;
using namespace geode::prelude;

namespace gdlutils {
    json loadJson(const std::string& name);
}