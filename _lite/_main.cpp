#include <Geode/Geode.hpp>
#include <Geode/ui/GeodeUI.hpp>
#include <alphalaneous.alphas_geode_utils/include/Utils.h>
using namespace geode::prelude;

#include "api.hpp"
namespace gdl {
    static std::unordered_map<std::string, std::unordered_map<Language, std::string>> apiStrings;
    Language getCurrentLanguage() {
        return (Language)Mod::get()->getSavedValue<int>("language-id", Language::GDL_RUSSIAN);
    }
    const char* getLanguageName(Language language) {
        switch (language) {
        case GDL_ENGLISH: return "English";
        case GDL_RUSSIAN: return "Русский";
        default: return "Unknown";
        }
    }
    const char* getLanguageCodename(Language language) {
        switch (language) {
        case GDL_ENGLISH: return "en";
        case GDL_RUSSIAN: return "ru";
        }
    }
    void addTranslation(const char* id, Language language, const char* translatedStr) {
        apiStrings[id][language] = translatedStr;
    }
    const char* getTranslation(const char* id, Language language) {
        if (apiStrings.contains(id) && apiStrings[id].contains(language)) {
            return apiStrings[id][language].c_str();
        } else return id;
    }
    void addTranslations(const char* id, std::initializer_list<std::pair<Language, const char*>> translations) {
        for (auto& translation : translations) addTranslation(id, translation.first, translation.second);
    }
    void addTranslationsFromFile(Language language, std::filesystem::path pathToJson) {
        auto read = file::readJson(pathToJson);
        for (auto& [k, v] : read.unwrapOrDefault()) {
            gdl::addTranslation(k.c_str(), language, v.asString().unwrapOrDefault().c_str());
        };
    }
}; // namespace gdl
const char* operator""_gdl(const char* str, size_t size) {
    return gdl::getTranslation(str, gdl::getCurrentLanguage());
}

#include <regex>

$execute { GEODE_WINDOWS(SetConsoleOutputCP(65001)); };

#include "Geode/modify/LoadingLayer.hpp"
class $modify(GDL_LoadingLayer, LoadingLayer) {
    static void removeUnsupportedSprites() {
        //потом (и кровью)
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eAdvFollowBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eAdvRandomBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eAlphaBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eAnimateBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eAreaFadeBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eAreaMoveBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eAreaRotateBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eAreaScaleBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eAreaStopBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eAreaTintBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eBGEOff_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eBGEOn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eBGSpeedBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eBPMBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eCParticleBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eCamGuideBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eCamModeBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eCamRotBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eChangeBG_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eChangeG_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eChangeMG_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eCollisionBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eCollisionStateBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eCountBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eCounterBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eEAreaFadeBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eEAreaMoveBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eEAreaRotateBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eEAreaScaleBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eEAreaTintBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eEdgeBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eEditAdvFollowBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eEditSFXBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eEditSongBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eEndBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eEndPlatBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eEnterFadeBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eEnterMoveBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eEnterRotateBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eEnterScaleBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eEnterStopBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eEnterTintBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eEventLinkBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eFollowComBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eFollowPComBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eGPOffsetBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eGhostDBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eGhostEBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eGradientBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eGravity_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eInstantCollisionBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eInstantCountBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eItemCompBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eItemEditBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eItemPersBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eKeyframeBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eLinkVisibleBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eMGSpeedBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eMoveComBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eOffsetBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eOnDeathBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eOptionsBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_ePHideBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_ePShowBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eParticleBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_ePauseBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_ePickupBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_ePlayerControlBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_ePulseBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eRandomBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eReAdvFollowBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eReleaseJumpBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eResetBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eResumeBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eReverseBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eRotateComBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eSFXBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eScaleComBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eSequenceBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eSetupMGBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eSh_BulgeBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eSh_ChromaticBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eSh_ChromaticGlitchBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eSh_EditColorBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eSh_GlitchBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eSh_GrayscaleBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eSh_HueBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eSh_InvertColorBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eSh_LensCircleBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eSh_MotionBlurBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eSh_PinchBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eSh_PixelateBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eSh_RadialBlurBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eSh_SepiaBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eSh_ShockLineBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eSh_ShockWaveBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eSh_SplitScreenBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eShaderBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eShakeBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eSongBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eSpawnBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eSpawnParticleBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eStartPosBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eStaticBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eStopMoverBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eTeleportBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eTimeBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eTimeControlBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eTimeEventBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eTimeWarpBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eTintCol01Btn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eToggleBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eTouchBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eUISettingsBtn_001"".png");
        CCSpriteFrameCache::get()->removeSpriteFrameByName("edit_eZoomBtn_001"".png");
    }
    static void loadResourcesForGDL() {

        auto tp = CCTexturePack();
        tp.m_paths = {string::pathToString(getMod()->getResourcesDir()).c_str()};
        tp.m_id = "resources"_spr;
        CCFileUtils::get()->addTexturePack(tp);

        auto langCode = gdl::getLanguageCodename(gdl::getCurrentLanguage());

        auto langFile = fmt::format("{}-lang.json", langCode);
        auto plistFile = fmt::format("GDL_TranslatedFrames-{}.plist", langCode);

        for (auto sp : CCFileUtils::get()->getSearchPaths()) {
            std::string path;

            path = (sp.c_str() + plistFile);
            if (CCFileUtils::get()->isFileExist(path.c_str())) {
                path = CCFileUtils::get()->fullPathForFilename(path.c_str(), 0).c_str();
                CCSpriteFrameCache::sharedSpriteFrameCache()->removeSpriteFramesFromFile(path.c_str());
                CCSpriteFrameCache::sharedSpriteFrameCache()->addSpriteFramesWithFile(path.c_str());
            }

            path = (sp.c_str() + langFile);
            if (CCFileUtils::get()->isFileExist(path.c_str())) {
                path = CCFileUtils::get()->fullPathForFilename(path.c_str(), 0).c_str();
                auto read = file::readJson(path);
                if (read.err()) {
                    auto err = read.err().value_or("unk err");
                    log::error("Failed to load localization file, {}", err);
                    gdl::addTranslation("Loading game resources", gdl::getCurrentLanguage(), ("Failed to load one of localization files...\n" + err).c_str());
                } 
                else for (auto& [k, v] : read.unwrapOrDefault()) {
                    gdl::addTranslation(k.c_str(), gdl::getCurrentLanguage(), v.asString().unwrapOrDefault().c_str());
                };
            }
        }

        removeUnsupportedSprites();
    }
    bool init(bool a) {
        loadResourcesForGDL();
        return LoadingLayer::init(a);
    }
};

enum TreeMapType {
    IDxNames,  // type##id
    TypeNames, // type
    IDs        // id or type
};

auto getNodeParentsTree(CCNode* node, TreeMapType mapType = TreeMapType::IDs) {
    auto map = std::map<std::string, unsigned int>(); // Используем std::string вместо const char*
    if (!node)
        return map; // Проверка на nullptr

    while (node->m_pParent) {
        auto name = AlphaUtils::Cocos::getClassName(node);
        auto id = node->getID(); // Исправлено: получаем ID правильно

        std::string key;
        switch (mapType) {
        case IDxNames:
            key = name + "##" + id;
            break;
        case TypeNames:
            key = name;
            break;
        case IDs:
            key = id.empty() ? name : id;
            break;
        }
        map[key] = map.size() + 1;
        node = node->m_pParent;
    }
    return map;
}

bool shouldUpdateWithTranslation(CCNode* node, const char* str) {
    if (!node || !str) return false; // nullptr любимый

    if (gdl::getTranslation(str) == std::string(str)) return false;

    auto parentsTree = getNodeParentsTree(node, TreeMapType::TypeNames);
    if (parentsTree.contains("CCTextInputNode")) return false;

    std::vector<std::string> wIDS = {
        "creator-name", 
        "level-name", 
        "username-button", 
        "song-name-label", 
        "artist-label",
    };

    std::string nodeID = node->getID();
    if (string::containsAny(nodeID, wIDS)) return false;

    if (auto parent = node->getParent())  {
        std::string parentID = parent->getID();
        if (string::containsAny(parentID, wIDS)) return false;
    }

    return true;
}

#include "Geode/modify/CCLabelBMFont.hpp"
class $modify(GDL_CCLabelBMFont, CCLabelBMFont) {
    void updateLocation() {
        static matjson::Value locations;

        auto file = fmt::format("{}-locations.json", gdl::getLanguageCodename(gdl::getCurrentLanguage()));
        if (!CCFileUtils::get()->m_fullPathCache.contains(file)) locations = file::readJson(
            CCFileUtils::get()->fullPathForFilename(file.c_str(), 0).c_str()
        ).unwrap();

        if (locations.contains(this->getString())) {
            auto loc = locations[this->getString()];
            if (loc.contains("scale")) setScale(loc["scale"].asDouble().unwrapOrDefault());
            if (loc.contains("x")) setPositionX(loc["x"].asDouble().unwrapOrDefault());
            if (loc.contains("y")) setPositionY(loc["y"].asDouble().unwrapOrDefault());
        };
    }

    bool tryUpdateWithTranslation(const char* str) {
        if (!str || !shouldUpdateWithTranslation(this, str)) return false;

        if (this->getString() != std::string(str)) return false;

        auto translation = gdl::getTranslation(str);

        auto point = typeinfo_cast<CCNode*>(this->getUserObject("translation-point"_spr));
        if (point) {
            if (translation != point->getID()) {
                this->setContentSize(point->getContentSize());
                this->setScale(point->getScale());
                this->setUserObject("translation-point"_spr, nullptr);
                return tryUpdateWithTranslation(str);
            }
        } 
        else {
            this->setString(str);
            auto size = this->getScaledContentSize();
            auto scale = this->getScale();

            this->setString(translation);
            limitNodeSize(this, size, this->getScale(), 0.1f);

            point = CCNode::create();
            if (point) { //кековщина
                point->setID(translation);
                point->setScale(scale);
                point->setContentSize(size);
                this->setUserObject("translation-point"_spr, point);
            }
        }
        return true;
    }

    bool initWithString(const char* pszstr, const char* font, float a3, cocos2d::CCTextAlignment a4, cocos2d::CCPoint a5) {
        if (!CCLabelBMFont::initWithString(pszstr, font, a3, a4, a5)) return false;
        // отложенная
        if (pszstr) {
            std::string str = pszstr; // паранорман
            queueInMainThread([__this = Ref(this), str] {
                if (__this && !str.empty()) {
                    __this->tryUpdateWithTranslation(str.c_str());
                }
            });
        }
        return true;
    }

    void setString(const char* newString) {
        CCLabelBMFont::setString(newString ? newString : "");
        this->tryUpdateWithTranslation(newString);
    }
};

#include "Geode/modify/MultilineBitmapFont.hpp"
class $modify(GDL_MultilineBitmapFont, MultilineBitmapFont) {
    struct Fields {
        float m_textScale = 1.0f;
        std::string m_fontName;
        float m_maxWidth = 0.0f;
    };

    static uint32_t nextUTF8(std::string::iterator& it, std::string::iterator end) {
        if (it >= end)
            return 0;

        unsigned char c1 = static_cast<unsigned char>(*it++);
        if (c1 < 0x80)
            return c1;

        if ((c1 & 0xE0) == 0xC0 && it < end) {
            unsigned char c2 = static_cast<unsigned char>(*it++);
            return ((c1 & 0x1F) << 6) | (c2 & 0x3F);
        }
        if ((c1 & 0xF0) == 0xE0 && it + 1 < end) {
            unsigned char c2 = static_cast<unsigned char>(*it++);
            unsigned char c3 = static_cast<unsigned char>(*it++);
            return ((c1 & 0x0F) << 12) | ((c2 & 0x3F) << 6) | (c3 & 0x3F);
        }
        if ((c1 & 0xF8) == 0xF0 && it + 2 < end) {
            unsigned char c2 = static_cast<unsigned char>(*it++);
            unsigned char c3 = static_cast<unsigned char>(*it++);
            unsigned char c4 = static_cast<unsigned char>(*it++);
            return ((c1 & 0x07) << 18) | ((c2 & 0x3F) << 12) | ((c3 & 0x3F) << 6) | (c4 & 0x3F);
        }
        return c1;
    }

    static void appendUTF8(uint32_t cp, std::string& str) {
        if (cp < 0x80) {
            str += static_cast<char>(cp);
        } else if (cp < 0x800) {
            str += static_cast<char>(0xC0 | (cp >> 6));
            str += static_cast<char>(0x80 | (cp & 0x3F));
        } else if (cp < 0x10000) {
            str += static_cast<char>(0xE0 | (cp >> 12));
            str += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
            str += static_cast<char>(0x80 | (cp & 0x3F));
        } else {
            str += static_cast<char>(0xF0 | (cp >> 18));
            str += static_cast<char>(0x80 | ((cp >> 12) & 0x3F));
            str += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
            str += static_cast<char>(0x80 | (cp & 0x3F));
        }
    }

#if !defined(GEODE_IS_IOS)
    gd::string readColorInfo(gd::string s) {
        std::string str = s;
        std::string str2;

        for (auto it = str.begin(); it != str.end();) {
            auto cp = nextUTF8(it, str.end());
            str2 += (cp < 128) ? static_cast<char>(cp) : 'W';
        }

        return MultilineBitmapFont::readColorInfo(str2);
    }
#endif

    bool initWithFont(const char* p0, gd::string p1, float p2, float p3, cocos2d::CCPoint p4, int p5, bool colorsDisabled) {
        if (!p0) return false;

        m_fields->m_textScale = p2;
        m_fields->m_fontName = p0;
        m_fields->m_maxWidth = p3;

        std::string translatedText = p1;
        if (shouldUpdateWithTranslation(this, p1.c_str())) {
            translatedText = gdl::getTranslation(p1.c_str());
        }

        std::string notags = translatedText;
        std::regex tagRegex("(<c.>)|(<\\/c>)|(<d...>)|(<s...>)|(<\\/s>)|(<i...>)|(<\\/i>)");
        notags = std::regex_replace(translatedText, tagRegex, "");

        if (!MultilineBitmapFont::initWithFont(p0, notags, p2, p3, p4, p5, true))
            return false;

        if (!colorsDisabled) {
            m_specialDescriptors = CCArray::create();
            if (!m_specialDescriptors)
                return false;

            m_specialDescriptors->retain();

            MultilineBitmapFont::readColorInfo(translatedText);

            if (m_specialDescriptors && m_characters) {
                for (auto i = 0u; i < m_specialDescriptors->count(); i++) {
                    auto tag = static_cast<TextStyleSection*>(m_specialDescriptors->objectAtIndex(i));
                    if (!tag) continue;

                    if (tag->m_endIndex == -1 && tag->m_styleType == TextStyleType::Delayed) {
                        if (tag->m_startIndex >= 0 && tag->m_startIndex < static_cast<int>(m_characters->count())) {
                            auto child = static_cast<CCFontSprite*>(m_characters->objectAtIndex(tag->m_startIndex));
                            if (child) {
                                child->m_fDelay = tag->m_delay;
                            }
                        }
                    } 
                    else {
                        int startIndex = std::max(0, tag->m_startIndex);
                        int endIndex = std::min(tag->m_endIndex, static_cast<int>(m_characters->count() - 1));

                        for (int j = startIndex; j <= endIndex && j >= 0; j++) {
                            if (j < static_cast<int>(m_characters->count())) {
                                auto child = static_cast<CCFontSprite*>(m_characters->objectAtIndex(j));
                                if (!child) continue;

                                switch (tag->m_styleType) {
                                case TextStyleType::Colored: {
                                    child->setColor(tag->m_color);
                                } break;
                                case TextStyleType::Instant: {
                                    child->m_bUseInstant = true;
                                    child->m_fInstantTime = tag->m_instantTime;
                                } break;
                                case TextStyleType::Shake: {
                                    child->m_nShakeIndex = j;
                                    child->m_fShakeIntensity = static_cast<float>(tag->m_shakeIntensity);
                                    child->m_fShakeElapsed = tag->m_shakesPerSecond <= 0 ? 0.0f : 1.0f / tag->m_shakesPerSecond;
                                } break;
                                default:
                                  break;
                                }
                            }
                        }
                    }
                }
            }

            if (m_specialDescriptors) {
                m_specialDescriptors->release();
                m_specialDescriptors = nullptr;
            }
        }

        return true;
    }

#if !defined(GEODE_IS_IOS)
    gd::string stringWithMaxWidth(gd::string p0, float scale, float scaledW) {
        auto width = m_fields->m_maxWidth;

        std::string translatedText = p0;
        if (shouldUpdateWithTranslation(this, p0.c_str())) {
            translatedText = gdl::getTranslation(p0.c_str());
        }

        std::string str = translatedText;
        if (auto pos = str.find('\n'); pos != std::string::npos) {
            str = str.substr(0, pos);
        }

        auto lbl = CCLabelBMFont::create("", m_fields->m_fontName.c_str());
        if (!lbl) return p0;

        lbl->setScale(m_fields->m_textScale);

        bool overflown = false;
        std::string current;
        for (auto it = str.begin(); it < str.end();) {
            auto cp = nextUTF8(it, str.end());
            appendUTF8(cp, current);

            lbl->setString(current.c_str());
            if (lbl->getScaledContentSize().width > width) {
                overflown = true;
                break;
            }
        }

        if (overflown) {
            if (auto pos = current.rfind(' '); pos != std::string::npos) {
                current.erase(current.begin() + pos, current.end());
            }
        } 
        else {
            current += " ";
        }

        return current;
    }
#endif
};

#include <Geode/modify/CCIMEDispatcher.hpp>
class $modify(GDL_CCIMEDispatcher, CCIMEDispatcher) {

    // почти нахуй не нужный фикс того как винда даёт буковки 
    // (чтоб работало в инпутах гд нужен обход фильтров)

    void dispatchInsertText(const char* text, int len, enumKeyCodes keys) {
#ifdef GEODE_IS_WINDOWS
        // только вызовы с длиной 1 и факт длиной 2 байта
        if (len == 1) {
            size_t actual_len = std::strlen(text);
            if (actual_len == 2) {
                // wchar_t из двух байтов
                wchar_t wch = (static_cast<unsigned char>(text[1]) << 8) | static_cast<unsigned char>(text[0]);

                // в utf8
                char utf8_buffer[8] = {0};
                int utf8_len = WideCharToMultiByte(CP_UTF8, 0, &wch, 1, utf8_buffer, sizeof(utf8_buffer), nullptr, nullptr);

                if (utf8_len > 0) {
                    // хых
                    return CCIMEDispatcher::dispatchInsertText(utf8_buffer, utf8_len, keys);
                }
            }
        }
#endif
        return CCIMEDispatcher::dispatchInsertText(text, len, keys);
    }
};