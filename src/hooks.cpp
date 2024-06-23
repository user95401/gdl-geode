#include "Geode/Geode.hpp"
#include "Geode/modify/MultilineBitmapFont.hpp"
#include "Geode/modify/CCLabelBMFont.hpp"
#include "Geode/modify/CCTextureCache.hpp"
#include "Geode/modify/LoadingLayer.hpp"

#include "utils.hpp"
#include <utf8.h>
#include <regex>
#include <filesystem>
#include <stringPatch.hpp>

#ifdef GEODE_IS_ANDROID
    #define __isascii isascii
#endif

using namespace geode::prelude;

class $modify(MultilineBitmapFont) {
    struct Fields {
        float m_textScale;
        std::string m_fontName;
        float m_maxWidth;
    };

    gd::string readColorInfo(gd::string s) {
        std::string str = s;

        std::string str2;
        for (auto it = str.begin(); it != str.end();) {
            auto cp = utf8::next(it, str.end());
            str2 += __isascii(cp) ? (char)cp : 'W';
        }

        return MultilineBitmapFont::readColorInfo(str2);
    }

    bool initWithFont(const char* p0, gd::string p1, float p2, float p3, cocos2d::CCPoint p4, int p5, bool colorsDisabled) {
        m_fields->m_textScale = p2;
        m_fields->m_fontName = p0;
        m_fields->m_maxWidth = p3;
        // log::debug("MBF;{};{};{};{}", m_fields->m_textScale, m_fields->m_fontName, (std::string)p1, p3);

        auto notags = std::regex_replace((std::string)p1, std::regex("(<c.>)|(<\\/c>)|(<d...>)|(<s...>)|(<\\/s>)|(<i...>)|(<\\/i>)"), "");
        if (!MultilineBitmapFont::initWithFont(p0, notags, p2, p3, p4, p5, true))
            return false;

        if (!colorsDisabled) {
            m_tagsArray = CCArray::create();
            m_tagsArray->retain();

            MultilineBitmapFont::readColorInfo(p1);

            for (auto i = 0u; i < m_tagsArray->count(); i++) {
                auto tag = (TextStyleSection*)(m_tagsArray->objectAtIndex(i));

                if (tag->m_end == -1 && tag->m_type == 4) {
                    auto child = (CCFontSprite*)m_lettersArray->objectAtIndex(tag->m_start);
                    if (child) {
                        child->m_delayTime = tag->m_delayTime;
                    }
                } else {
                    for (auto i = tag->m_start; i <= tag->m_end; i++) {
                        auto child = (CCFontSprite*)(m_lettersArray->objectAtIndex(i));
                        if (!child)
                            continue;

                        switch (tag->m_type) {
                        case 1: {
                            child->setColor(tag->m_col);
                        } break;
                        case 2: {
                            child->m_isInstant = true;
                            child->m_instantValue = tag->m_instantNum;
                        } break;
                        case 3: {
                            child->m_thisTagNumber = i;
                            child->m_shakeVal1 = (float)tag->m_shakeNum1;
                            child->m_shakeVal2 = tag->m_shakeNum2 <= 0 ? 0.0f : 1.0f / tag->m_shakeNum2;
                        } break;
                        default:
                            break;
                        }
                    }
                }
            }

            m_tagsArray->release();
            m_tagsArray = nullptr;
        }

        return true;
    }

    gd::string stringWithMaxWidth(gd::string p0, float scale, float scaledW) {
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
        } else {
            current += " ";
        }

        return current;
    }
};

class $modify(CCTextureCache) {
    cocos2d::CCTexture2D* addImage(const char* name, bool idk) {
        auto newName = (Mod::get()->getResourcesDir() / name).string();

        if (std::filesystem::exists(newName)) {
#ifdef GEODE_IS_WINDOWS
            return CCTextureCache::addImage(std::filesystem::relative(newName).string().c_str(), idk);
#else
            return CCTextureCache::addImage(newName.c_str(), idk);
#endif
        }
        
        return CCTextureCache::addImage(name, idk);
    }
};

class $modify(CCLabelBMFont) {
    bool initWithString(char const* str, char const* font, float a3, cocos2d::CCTextAlignment a4, cocos2d::CCPoint a5) {
        auto newName = Mod::get()->getResourcesDir() / font;

        if (std::filesystem::exists(newName)) {
#ifdef GEODE_IS_WINDOWS
            return CCLabelBMFont::initWithString(str, std::filesystem::relative(newName).string().c_str(), a3, a4, a5);
#else
            return CCLabelBMFont::initWithString(str, gdlutils::pathWithQuality(newName).c_str(), a3, a4, a5);
#endif
        }

        return CCLabelBMFont::initWithString(str, font, a3, a4, a5);
    }
};

class $modify(LoadingLayer){
    void loadAssets(){
        if(this->m_loadStep == 11 && gdl::getCurrentLanguage() != gdl::GDL_ENGLISH){
            auto plist = Mod::get()->getResourcesDir() / fmt::format("GDL_TranslatedFrames-{}.plist", gdl::getLanguageCodename(gdl::getCurrentLanguage()));
            auto png = Mod::get()->getResourcesDir() / fmt::format("GDL_TranslatedFrames-{}.png", gdl::getLanguageCodename(gdl::getCurrentLanguage()));

            CCSpriteFrameCache::sharedSpriteFrameCache()->removeSpriteFramesFromFile(gdlutils::pathWithQuality(plist).c_str());
            CCSpriteFrameCache::sharedSpriteFrameCache()->addSpriteFramesWithFile(gdlutils::pathWithQuality(plist).c_str(),
                CCTextureCache::sharedTextureCache()->addImage(gdlutils::pathWithQuality(png).c_str(), false)
            );
        }

        LoadingLayer::loadAssets();
    }
};

#ifdef GEODE_IS_WINDOWS

void (__thiscall* gd_string_assign_o)(void* self, char* src, size_t len);
void gd_string_assign_hk(void* self, char* src, size_t len) {
/*     
        Зачем нужен этот хук:
        В некоторых случаях в гд, функция GDString::assign вызывается в заданной длинной,
        которая ограничивает длинну строки, но переведенная строка может быть длиннее чем оригинальная.
        Было бы намного сложнее пропатчить все длины строк, учитывая то, что не везде указывается длина строки.
        В общем, намного легче хукнуть эту функцию и задать новую длину строки.

        Why this is required:
        In some places of gd you can see GDString::assign called with fixed length
        which limits strings to the length it was in the game but the translated strings may be longer than the original ones.
        It is also pretty difficult to patch the length as it is not used in all calls and i would somehow need to find places to patch
        So it is easier to hook it and just set it to the string length instead of the fixed length
*/
    gd_string_assign_o(self, src, strlen(src));
}

// void (__thiscall* gd_string_append_o)(void* self, char* src, size_t len);
// void gd_string_append_hk(void* self, char* src, size_t len) {
//     // Same as for gd::string::assign

// 	gd_string_append_o(self, src, strlen(src));
// }

#endif

$execute {
#ifdef GEODE_IS_WINDOWS
    constexpr auto GD_STR_ASSIGN_ADDR = 0x3BE50;
    gd_string_assign_o = reinterpret_cast<void (__thiscall*)(void* self, char* src, size_t len)>(base::get() + GD_STR_ASSIGN_ADDR);
    auto res2 = Mod::get()->hook((void*)(base::get() + GD_STR_ASSIGN_ADDR), gd_string_assign_hk, "gd::string::assign", tulip::hook::TulipConvention::Thiscall).err();
    if (res2 != std::nullopt) {
        log::error("Failed to hook gd::string::assign because of: {}", res2);
    }
    
    // constexpr auto GD_STR_APPEND_ADDR = 0x21DE0;
    // gd_string_append_o = reinterpret_cast<void (__thiscall*)(void*, char*, size_t)>(base::get() + GD_STR_APPEND_ADDR);
    // auto res3 = Mod::get()->hook((void*)(base::get() + GD_STR_APPEND_ADDR), gd_string_append_hk, "gd::string::append", tulip::hook::TulipConvention::Thiscall).err();
    // if (res3 != std::nullopt) {
    //     log::error("Failed to hook gd::string::append because of: {}", res3);
    // }
#endif
}