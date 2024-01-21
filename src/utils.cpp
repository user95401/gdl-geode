#include "utils.hpp"

#include <Geode/Geode.hpp>

using namespace geode::prelude;

#define CURRENT_LOC "ru"

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

std::string gdlutils::getQualityString(std::string filename) {
    if (filename.find("-uhd") != std::string::npos)
        filename.erase(filename.find("-uhd"), 4);
    if (filename.find("-hd") != std::string::npos)
        filename.erase(filename.find("-hd"), 3);

    std::string extension = ghc::filesystem::path(filename).extension().string();
    auto quality = CCDirector::get()->getLoadedTextureQuality();

    switch (quality) {
    case TextureQuality::kTextureQualityLow:
        break;

    case TextureQuality::kTextureQualityMedium:
        filename.replace(filename.find(extension), std::string("-hd" + extension).length(), "-hd" + extension);
        break;

    case TextureQuality::kTextureQualityHigh:
        filename.replace(filename.find(extension), std::string("-uhd" + extension).length(), "-uhd" + extension);
        break;
    }

    return filename;
}

// ghc::filesystem::path gdlutils::getLocPath() {
//     return Mod::get()->getResourcesDir() / "loc" / CURRENT_LOC;
// }

// ghc::filesystem::path gdlutils::getRelativeResPath() {
//     // auto curPath = ghc::filesystem::current_path();
//     // auto resPath = Mod::get()->getResourcesDir();
//     // return resPath.relative_path();
// }
