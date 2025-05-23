#include <Geode/Geode.hpp>
#include <alphalaneous.alphas_geode_utils/include/Utils.h>
#include "api.hpp"

#include "_updater.hpp"

#include <regex>
#include <utf8.h>

using namespace geode::prelude;

#if defined(GEODE_IS_ANDROID) || defined(__APPLE__)
#define __isascii isascii
#endif

$execute{
    GEODE_WINDOWS(SetConsoleOutputCP(65001)); // utf8

    //не ну а хули нет
    CCFileUtils::get()->addPriorityPath(getMod()->getResourcesDir().string().c_str());

    auto code = gdl::getLanguageCodename(gdl::getCurrentLanguage());
    auto lang_set = fmt::format("/{}-lang.json", code);

    auto sps = CCFileUtils::get()->getSearchPaths();
    for (auto sp : sps) {
        auto file = (sp.c_str() + lang_set);
        if (!CCFileUtils::get()->isFileExist(file.c_str())) continue;
        gdl::addTranslationsFromFile(gdl::getCurrentLanguage(), file.c_str());
    }

};
#include "Geode/modify/LoadingLayer.hpp"
class $modify(LoadingLayer) {
    $override bool init(bool a) {
        geodeExecFunction16<LoadingLayer>();
        return LoadingLayer::init(a);
    }
    $override void loadAssets() {
        LoadingLayer::loadAssets();
        if (this->m_loadStep == 11) {
            geodeExecFunction16<LoadingLayer>();
            if (gdl::getCurrentLanguage() != gdl::GDL_ENGLISH) {
                auto plist = fmt::format("GDL_TranslatedFrames-{}.plist", gdl::getLanguageCodename(gdl::getCurrentLanguage()));
                auto png = fmt::format("GDL_TranslatedFrames-{}.png", gdl::getLanguageCodename(gdl::getCurrentLanguage()));
                CCSpriteFrameCache::sharedSpriteFrameCache()->removeSpriteFramesFromFile(plist.c_str());
                CCSpriteFrameCache::sharedSpriteFrameCache()->addSpriteFramesWithFile(plist.c_str(), png.c_str());
            };
        }
    }
};

enum TreeMapType {
    IDxNames, //type##id
    TypeNames, //type
    IDs //id or type
};
auto getNodeParentsTree(CCNode* node, TreeMapType mapType = TreeMapType::IDs) {
    auto map = std::map<const char*, unsigned int>();
    if (node) while (node->m_pParent) {
        auto name = AlphaUtils::Cocos::getClassName(node);
        auto id = AlphaUtils::Cocos::getClassName(node);
        switch (mapType) {
        case IDxNames: map[(name + "##" + id).c_str()] = map.size() + 1;
            break;
        case TypeNames: map[(name).c_str()] = map.size() + 1;
            break;
        case IDs: map[(id.empty() ? name : id).c_str()] = map.size() + 1;
            break;
        }
        node = node->m_pParent;
    }
    log::debug("{} -> {}", __FUNCTION__, map);
    return map;
}
bool shouldUpdateWithTranslation(CCNode* node, const char* str) {
    //про верочек
    if (!node) return false;
    if (!gdl::hasTranslation(str)) return false;
    if (getNodeParentsTree(node, TreeMapType::TypeNames).contains("CCTextInputNode")) return false;
    std::vector<std::string> wIDS = {
        "creator-name"
        ,"level-name"
        ,"username-button"
        ,"song-name-label"
        ,"artist-label"
    };
    if (string::containsAny(node->getID(), wIDS)) return false;
    if (node->getParent() and string::containsAny(node->getParent()->getID(), wIDS)) return false;
    return true;
}

#include "Geode/modify/CCLabelBMFont.hpp"
class $modify(CCLabelBMFontTranslation, CCLabelBMFont) {
    void updateLocation() {
        if (gdl::getLocations().contains(this->getString())) {
            auto loc = gdl::getLocations()[this->getString()];
            if (loc.contains("scale")) setScale(loc["scale"].asDouble().unwrapOrDefault());
            if (loc.contains("x")) setPositionX(loc["x"].asDouble().unwrapOrDefault());
            if (loc.contains("y")) setPositionY(loc["y"].asDouble().unwrapOrDefault());
        };
    }
    bool tryUpdateWithTranslation(char const* str) {
        if (!shouldUpdateWithTranslation(this, str)) return false;
        auto translation = gdl::getTranslation(str);
        if (auto point = typeinfo_cast<CCNode*>(this->getUserObject("translation-point"_spr))) {
            if (translation != point->getID()) {
                this->setContentSize(point->getContentSize());
                this->setScale(point->getScale());
                this->setUserObject("translation-point"_spr, nullptr);
                CC_SAFE_RELEASE_NULL(point);
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
            point->setID(translation);
            point->setScale(scale);
            point->setContentSize(size);
            this->setUserObject("translation-point"_spr, point);//save "changed flag"

            updateLocation();
        };
        return true;
    }
    bool initWithString(char const* str, char const* font, float a3, cocos2d::CCTextAlignment a4, cocos2d::CCPoint a5) {
        if (!CCLabelBMFont::initWithString(str, font, a3, a4, a5)) return false;
        queueInMainThread([__this = Ref(this)] { if (__this) __this->tryUpdateWithTranslation(__this->getString()); });
        return true;
    }
    void setString(const char* newString) {
        //какого хуя оно постоянно вызывается пока нод живёт
        if (!tryUpdateWithTranslation(newString)) return CCLabelBMFont::setString(newString);
    }
};

#if defined(__APPLE__)
#else
#include "Geode/modify/MultilineBitmapFont.hpp"
class $modify(MultilineBitmapFont) {
    struct Fields {
        float m_textScale;
        std::string m_fontName;
        float m_maxWidth;
    };
    $override gd::string readColorInfo(gd::string s) {
        std::string str = s;

        std::string str2;
        for (auto it = str.begin(); it != str.end();) {
            auto cp = utf8::next(it, str.end());
            str2 += __isascii(cp) ? (char)cp : 'W';
        }

        return MultilineBitmapFont::readColorInfo(str2);
    }
    $override bool initWithFont(const char* p0, gd::string p1, float p2, float p3, cocos2d::CCPoint p4, int p5, bool colorsDisabled) {
        if (shouldUpdateWithTranslation(this, p1.c_str())) p1 = gdl::getTranslation(p1.c_str());

        // log::debug("string!!! {} {} {}", p1.size(), p1.capacity(), p1.c_str());
        m_fields->m_textScale = p2;
        m_fields->m_fontName = p0;
        m_fields->m_maxWidth = p3;
        // log::debug("MBF;{};{};{};{}", m_fields->m_textScale, m_fields->m_fontName, (std::string)p1, p3);

        auto notags = std::regex_replace((std::string)p1, std::regex("(<c.>)|(<\\/c>)|(<d...>)|(<s...>)|(<\\/s>)|(<i...>)|(<\\/i>)"), "");
        if (!MultilineBitmapFont::initWithFont(p0, notags, p2, p3, p4, p5, true))
            return false;

        if (!colorsDisabled) {
            m_specialDescriptors = CCArray::create();
            m_specialDescriptors->retain();

            MultilineBitmapFont::readColorInfo(p1);

            for (auto i = 0u; i < m_specialDescriptors->count(); i++) {
                auto tag = (TextStyleSection*)(m_specialDescriptors->objectAtIndex(i));

                if (tag->m_endIndex == -1 && tag->m_styleType == TextStyleType::Delayed) {
                    auto child = (CCFontSprite*)m_characters->objectAtIndex(tag->m_startIndex);
                    if (child) {
                        child->m_fDelay = tag->m_delay;
                    }
                }
                else {
                    for (auto i = tag->m_startIndex; i <= tag->m_endIndex; i++) {
                        auto child = (CCFontSprite*)(m_characters->objectAtIndex(i));
                        if (!child)
                            continue;

                        switch (tag->m_styleType) {
                        case TextStyleType::Colored: {
                            child->setColor(tag->m_color);
                        } break;
                        case TextStyleType::Instant: {
                            child->m_bUseInstant = true;
                            child->m_fInstantTime = tag->m_instantTime;
                        } break;
                        case TextStyleType::Shake: {
                            child->m_nShakeIndex = i;
                            child->m_fShakeIntensity = (float)tag->m_shakeIntensity;
                            child->m_fShakeElapsed = tag->m_shakesPerSecond <= 0 ? 0.0f : 1.0f / tag->m_shakesPerSecond;
                        } break;
                        default:
                            break;
                        }
                    }
                }
            }

            m_specialDescriptors->release();
            m_specialDescriptors = nullptr;
        }

        return true;
    };
    $override gd::string stringWithMaxWidth(gd::string p0, float scale, float scaledW) {
        p0 = gdl::getTranslation(p0.c_str());
        
        auto width = m_fields->m_maxWidth;

        std::string str = p0;
        if (auto pos = str.find('\n'); pos != std::string::npos) {
            str = str.substr(0, pos);
        }

        auto lbl = CCLabelBMFont::create("", m_fields->m_fontName.c_str());
        lbl->setScale(m_fields->m_textScale);

        bool overflown = false;
        std::string current;
        for (auto it = str.begin(); it < str.end();) {
            auto cp = utf8::next(it, str.end());
            utf8::append(cp, current);

            // auto x = cocos2d::FNTConfigLoadFile("chatFont.fnt");
            // auto y = x->m_pFontDefDictionary;

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
};
#endif

#include <Geode/modify/OptionsLayer.hpp>
#include "LanguageLayer.hpp"
class $modify(GDLOptionsLayer, OptionsLayer) {
     struct Fields {
         bool m_clickedLanguage = false;
     };

     void onLanguage(CCObject* sender) {
         m_fields->m_clickedLanguage = true;
         this->hideLayer(false);
     }

     virtual void customSetup() {
         OptionsLayer::customSetup();

         auto buttonsMenu = (CCMenu*)this->m_mainLayer->getChildren()->objectAtIndex(4);
         if(!buttonsMenu)
             return;

         auto languageBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("language"_gdl, "goldFont.fnt", "GJ_button_01.png"), this, SEL_MenuHandler(&GDLOptionsLayer::onLanguage));
         languageBtn->setPosition(0, -115);
         buttonsMenu->addChild(languageBtn);
     }

     virtual void layerHidden() {
         if(m_fields->m_clickedLanguage) {
             auto languageLayer = LanguageLayer::create();
             this->getParent()->addChild(languageLayer);
             languageLayer->setZOrder(100);
             languageLayer->showLayer(false);
             m_fields->m_clickedLanguage = false;
         }

         OptionsLayer::layerHidden();
     }
 };

