#include "Geode/Geode.hpp"
#include "Geode/modify/MultilineBitmapFont.hpp"
#include "Geode/modify/CCLabelBMFont.hpp"
#include "Geode/modify/CCTextureCache.hpp"

#include "utils.hpp"
#include <utf8.h>
#include <regex>

using namespace geode::prelude;

class $modify(MultilineBitmapFont) {
    float m_textScale;
    std::string m_fontName;

    gd::string readColorInfo(gd::string s) {
        log::debug("color;{}", s);

        std::string str = s;

        std::string str2;
        for (auto it = str.begin(); it != str.end();) {
            auto cp = utf8::next(it, str.end());
            str2 += isascii(cp) ? (char)cp : 'E';
        }
        
        auto ret = MultilineBitmapFont::readColorInfo(str2);
        log::debug("color ret;{}", ret);

        str = std::regex_replace(str, std::regex("<c.>"), "");
        str = std::regex_replace(str, std::regex("</c>"), "");
        str = std::regex_replace(str, std::regex("<d...>"), "");
        str = std::regex_replace(str, std::regex("<s...>"), "");
        str = std::regex_replace(str, std::regex("</s>"), "");
        str = std::regex_replace(str, std::regex("<i>"), "");
        str = std::regex_replace(str, std::regex("</i>"), "");
        
        return str;
    }
    
    bool initWithFont(char const* p0, gd::string p1, float p2, float p3, cocos2d::CCPoint p4, int p5, bool p6) {
        log::debug("init;{};{};{};{};{} {};{};{}", p0, p1, p2, p3, p4.x, p4.y, p5, p6);
        m_fields->m_textScale = p2;
        m_fields->m_fontName = p0;
        return MultilineBitmapFont::initWithFont(p0,p1,p2,p3,p4,p5,p6);
    }

    gd::string stringWithMaxWidth(gd::string p0, float scale, float scaledW) {
        auto width = scaledW / CCDirector::sharedDirector()->getContentScaleFactor();
        log::debug("SWMW;{};scale:{};scaledW:{};width:{}", p0, scale, scaledW, width);

        std::string str = p0;
        if (auto pos = str.find('\n'); pos != std::string::npos) {
            str = str.substr(0, pos);
        }

        log::info("SWMW!!;{};{}", m_fields->m_textScale, m_fields->m_fontName);
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

        current += "\n";
        return current;
    }
};

cocos2d::CCBMFontConfiguration* FNTConfigLoadFile_hk(char const* name) {
    auto newName = Mod::get()->getResourcesDir() / name;
    if (ghc::filesystem::exists(newName)) {
        return cocos2d::FNTConfigLoadFile(ghc::filesystem::relative(newName).string().c_str());
    }
    return cocos2d::FNTConfigLoadFile(name);
}

class $modify(CCTextureCache) {
    cocos2d::CCTexture2D* addImage(const char* name, bool idk) {
        auto newName = Mod::get()->getResourcesDir() / name;
        if (ghc::filesystem::exists(newName)) {
            return CCTextureCache::addImage(ghc::filesystem::relative(newName).string().c_str(), idk);
        }
        return CCTextureCache::addImage(name, idk);
    }
};

void (__thiscall* gd_string_assign_o)(void* self, char* src, size_t len);
void gd_string_assign_hk(void* self, char* src, size_t len) {
/*     
        Зачем нужны эти хуки:
        В некоторых случаях в гд, функция GDString::assign вызывается в заданной длинной,
        которая ограничивает длинну строки, но переведенная строка может быть длиннее чем оригинальная.
        Было бы намного сложнее пропатчить все длины строк, учитывая то, что не везде указывается длина строки.
        В общем намного легче хукнуть эту функция и установить новую длину строки.

        Why this is required:
        In some places of gd you can see GDString::assign called with fixed length
        which limits strings to the length it was in the game but the translated strings may be longer than the original ones.
        It is also pretty difficult to patch the length as it is not used in all calls and i would somehow need to find places to patch
        So it is easier to hook it and just set it to the string length instead of the fixed length
*/
	gd_string_assign_o(self, src, strlen(src));
}

$execute {
    auto addr = GetProcAddress(GetModuleHandleA("libcocos2d.dll"), "?FNTConfigLoadFile@cocos2d@@YAPAVCCBMFontConfiguration@1@PBD@Z");
    auto res1 = Mod::get()->hook((void*)addr, FNTConfigLoadFile_hk, "cocos2d::FNTConfigLoadFile", tulip::hook::TulipConvention::Cdecl).err();
    if (res1 != std::nullopt) {
        log::error("Failed to hook cocos2d::FNTConfigLoadFile because of: {}", res1);
    }
    
    constexpr auto GD_STR_ASSIGN_ADDR = 0x1BB10;
    gd_string_assign_o = reinterpret_cast<void (__thiscall*)(void* self, char* src, size_t len)>(base::get() + GD_STR_ASSIGN_ADDR);
    auto res2 = Mod::get()->hook((void*)(base::get() + GD_STR_ASSIGN_ADDR), gd_string_assign_hk, "gd::string::assign", tulip::hook::TulipConvention::Thiscall).err();
    if (res2 != std::nullopt) {
        log::error("Failed to hook gd::string::assign because of: {}", res1);
    }
}