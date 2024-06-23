#include "utils.hpp"
#include <Geode/Geode.hpp>

using namespace geode::prelude;

json gdlutils::loadJson(const std::string& name) {
    std::ifstream translationFileStream(name);

    if (translationFileStream.is_open()) {
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

void gdlutils::reloadAll() {
	CCDirector::sharedDirector()->updateContentScale(CCDirector::get()->getLoadedTextureQuality());

	auto gameManager = GameManager::sharedState();
	// gameManager->setQuality(CCDirector::get()->getLoadedTextureQuality());
	CCTexture2D::setDefaultAlphaPixelFormat(kCCTexture2DPixelFormat_RGBA4444);
	gameManager->reloadAll(false, false, true);
}