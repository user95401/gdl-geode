#pragma once
#include <string>
#include <json/json.hpp>
#include <filesystem>

using namespace nlohmann;

struct ModRM {
    uint8_t size;
    uint8_t reg;
};

namespace gdlutils {
    json loadJson(const std::string& name);
    std::string pathWithQuality(const std::filesystem::path& path);
    void reloadAll();

    ModRM decodeModRM(uint8_t modrm);
}