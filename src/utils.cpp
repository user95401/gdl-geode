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

ModRM gdlutils::decodeModRM(uint8_t modrm){
    ModRM ret;

    // ModRM 
    // Mod (2 bits) | Reg (3 bits) | RM (3 bits)
    // Firstly we need mod to recognize size of instuction

    switch(modrm >> 6) {
        case 0b00:
        case 0b11: ret.size = 2; break;
        case 0b01: ret.size = 3; break;
        case 0b10: ret.size = 4; break;
    }
    
    ret.reg = (modrm >> 2) & 0b00000111;

    return ret
}