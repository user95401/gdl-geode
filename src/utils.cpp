#include "utils.hpp"

std::vector<std::string> gdlutils::splitString(const std::string& str, char separator) {
    std::string temp = "";
    std::vector<std::string> v;

    for (size_t i = 0; i < str.length(); ++i) {
        if (str[i] == separator) {
            v.push_back(temp);
            temp = "";
        } else {
            temp.push_back(str[i]);
        }
    }
    v.push_back(temp);

    return v;
}

std::string gdlutils::joinStrings(std::vector<std::string> strings, const char* delim) {
    std::string ret;

    for (size_t i = 0; i < strings.size(); i++) {
        auto str = strings[i];
        ret += (i != strings.size() - 1) ? str + delim : str;
    }

    return ret;
}

size_t gdlutils::sequenceLength(uint8_t byte) {
    if (byte >> 7 == 0b0)
        return 1;
    else if (byte >> 5 == 0b110)
        return 2;
    else if (byte >> 4 == 0b1110)
        return 3;
    else if (byte >> 3 == 0b11110)
        return 4;
    return 0;
}

std::string gdlutils::replaceUnicode(const std::string& str) {
    std::string ret = "";

    auto ptr = str.c_str();
    while (*ptr) {
        auto len = sequenceLength(*ptr);
        ret += len > 1 ? 'E' : *ptr; // it should be a 1-byte so it doesnt mess up with counting symbols
        ptr += len;
    }

    return ret;
}

std::vector<std::string> gdlutils::splitByWidth(const std::string& src, float width, const char* fontName) {
    std::vector<std::string> ret;

    std::string str = src;

    while (str.size()) {
        auto lbl = CCLabelBMFont::create("", fontName);

        auto hasNL = str.find("\n") != std::string::npos;
        auto line = hasNL ? splitString(str, '\n')[0] : str;

        bool overflown = false;
        std::string current;

        auto ptr = line.c_str();

        while (*ptr) {
            auto len = sequenceLength(*ptr);
            auto letter = std::string(ptr, len);
            current += letter;
            ptr += len;

            lbl->setString(current.c_str());

            if (lbl->getScaledContentSize().width > width) {
                overflown = true;
                break;
            }
        }

        if (overflown) {
            if (current.find(' ') != std::string::npos) {
                auto words = splitString(current, ' ');
                words.pop_back();
                current = joinStrings(words, " ");
                current = joinStrings(words, " ") + " ";
            }
        } else if (hasNL) {
            current += " ";
        }

        ret.push_back(current);
        str = str.erase(0, current.size());
    }

    return ret;
}

nlohmann::json gdlutils::loadJson(const std::string& name) {
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

bool gdlutils::shouldReverseGauntlet(int id) {
    return id == 4 ||  // Shadow
           id == 5 ||  // Lava
           id == 7 ||  // Chaos
           id == 9 ||  // Time
           id == 11 || // Magic
           id == 12 || // Spike
           id == 13 || // Monster
           id == 14 || // Doom
           id == 15;   // Death
}

std::string gdlutils::getQualityString(std::string filename){
    if(filename.find("-uhd") != std::string::npos) filename.erase(filename.find("-uhd"), 4);
    if(filename.find("-hd") != std::string::npos) filename.erase(filename.find("-hd"), 3);

    std::string extension = ghc::filesystem::path(filename).extension().string();
    auto quality = CCDirector::get()->getLoadedTextureQuality();

    switch (quality){
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

CCMenuItem* gdlutils::createMenuProfile(char const* name, char const* spriteFrameName, bool smallNick){
    auto item = CCMenuItem::create();
    item->addChild(CCSprite::createWithSpriteFrameName(spriteFrameName));
    
    auto textProfile = CCLabelBMFont::create(name, "goldFont.fnt");
    textProfile->setPositionY((smallNick) ? -23.5f : -23.0f);
    textProfile->limitLabelWidth(30.f, 0.5f, 0.35f);
    item->addChild(textProfile);

    return item;
}

void gdlutils::reloadAll(){
	CCDirector::sharedDirector()->updateContentScale(CCDirector::get()->getLoadedTextureQuality());

	auto gameManager = GameManager::sharedState();
	// gameManager->setQuality(CCDirector::get()->getLoadedTextureQuality());
	CCTexture2D::setDefaultAlphaPixelFormat(kCCTexture2DPixelFormat_RGBA4444);
	gameManager->reloadAll(false, false, true);
}

void gdlutils::achievementsTranslation(bool enable){
#ifdef GEODE_IS_WINDOWS
    const char* plist = (enable) ? "AchievementsDesc.plist"_spr : "AchievementsDesc.plist";
    gdlutils::patchString(base::get() + 0x7BD9, plist);
#endif
}


#ifdef GEODE_IS_WINDOWS
void gdlutils::patchString(uintptr_t absAddress, char const* str) {
    Mod::get()->patch((void*)absAddress, ByteVector {(uint8_t*)&str, (uint8_t*)&str + 4});
}
#elif defined(GEODE_IS_ANDROID)
void gdlutils::patchString(uintptr_t dcd, uintptr_t add, char const* str) {
    Mod::get()->patch((void*)(base::get() + dcd), ByteVector {(uint8_t*)&str, (uint8_t*)&str + 4});
    Mod::get()->patch((void*)(base::get() + add), ByteVector {0x00, 0xBF});
}
#endif