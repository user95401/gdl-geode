#pragma once
#include <string>
#include <json/json.hpp>
#include <Geode/Geode.hpp>

using namespace nlohmann;
using namespace geode::prelude;

namespace gdlutils {
    json loadJson(const std::string& name);
    std::string getQualityString(std::string filename);
    // ghc::filesystem::path getLocPath();
    // ghc::filesystem::path getRelativeResPath();
}