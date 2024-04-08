#include "utils.hpp"
#include <Geode/Geode.hpp>

using namespace geode::prelude;

json gdlutils::loadJson(const std::string& name) {
    std::ifstream translationFileStream(name);

    if (translationFileStream) {
        try {
            nlohmann::json translationObj;
            translationFileStream >> translationObj;

            return translationObj;
        } catch (...) {
            log::error("Failed to parse json \"{}\". Please check the file for mistakes.", name);

            return nullptr;
        }
    } else {
        log::error("Failed to open json \"{}\".", name);

        return nullptr;
    }
}

std::string gdlutils::pathWithQuality(const std::filesystem::path& path) {
    auto quality = CCDirector::get()->getLoadedTextureQuality();

    auto str = path.parent_path() / path.stem();
    auto ext = path.extension();

    switch (quality) {
        case TextureQuality::kTextureQualityMedium:
            return str.string() + "-hd" + ext.string();
        case TextureQuality::kTextureQualityHigh:
            return str.string() + "-uhd" + ext.string();
        default:
            return path.string();
    }
}
