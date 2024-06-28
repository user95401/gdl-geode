#pragma once
#include <string>
#include <json/json.hpp>
#include <filesystem>

using namespace nlohmann;

namespace gdlutils {
    json loadJson(const std::string& name);
    std::string pathWithQuality(const std::filesystem::path& path);
    void reloadAll();
}