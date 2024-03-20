#include "Geode/Geode.hpp"
#include "Geode/modify/MultilineBitmapFont.hpp"
#include "Geode/modify/CCLabelBMFont.hpp"
#include "Geode/modify/CCTextureCache.hpp"

#include "utils.hpp"
#include <utf8.h>
#include <regex>
#include <filesystem>

using namespace geode::prelude;

class CCFontSprite : public CCSprite {
  public:
	PAD(8);
	bool m_isInstant;
	float m_instantValue;
	float m_delayTime;
	float m_shakeVal2;
	float m_shakeVal1;
    PAD(4);
    int m_thisTagNumber;
};

class $modify(MultilineBitmapFont) {
    float m_textScale;
    std::string m_fontName;

    gd::string readColorInfo(gd::string s) {
        std::string str = s;

        std::string str2;
        for (auto it = str.begin(); it != str.end();) {
            auto cp = utf8::next(it, str.end());
            str2 += isascii(cp) ? (char)cp : 'W';
        }

        auto orig = MultilineBitmapFont::readColorInfo(str2);

        return orig;
    }

    bool initWithFont(const char* p0, gd::string p1, float p2, float p3, cocos2d::CCPoint p4, int p5, bool p6) {
        // log::debug("init;{};{};{};{};{} {};{};{}", p0, p1, p2, p3, p4.x, p4.y, p5, p6);
        m_fields->m_textScale = p2;
        m_fields->m_fontName = p0;

        auto notags = std::regex_replace((std::string)p1, std::regex("(<c.>)|(<\\/c>)|(<d...>)|(<s...>)|(<\\/s>)|(<i...>)|(<\\/i>)"), "");
        if (!MultilineBitmapFont::initWithFont(p0, notags, p2, p3, p4, p5, true))
            return false;

        if (!p6) {
            m_tagsArray = CCArray::create();
            m_tagsArray->retain();

            MultilineBitmapFont::readColorInfo(p1);

            // log::debug("{} tags", m_tagsArray->count());
            for (auto i = 0u; i < m_tagsArray->count(); i++) {
                auto tag = (TextStyleSection*)(m_tagsArray->objectAtIndex(i));
                // log::debug("!!!tag {} ; {} - {}", tag->m_type, tag->m_start, tag->m_end);

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
                            // log::debug("SHAKE;{} {}", tag->m_shakeNum1, tag->m_shakeNum2);
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
        auto width = scaledW / CCDirector::sharedDirector()->getContentScaleFactor();

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

        current += " ";
        return current;
    }
};

cocos2d::CCBMFontConfiguration* FNTConfigLoadFile_hk(char const* name) {
    log::info("FNT {}", name);
    auto newName = (Mod::get()->getResourcesDir() / name).string();
    if (std::filesystem::exists(newName)) {
        return cocos2d::FNTConfigLoadFile(std::filesystem::relative(newName).string().c_str());
    }
    return cocos2d::FNTConfigLoadFile(name);
}

class $modify(CCTextureCache) {
    cocos2d::CCTexture2D* addImage(const char* name, bool idk) {
        auto newName = (Mod::get()->getResourcesDir() / name).string();
        if (std::filesystem::exists(newName)) {
            return CCTextureCache::addImage(std::filesystem::relative(newName).string().c_str(), idk);
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
        log::error("Failed to hook gd::string::assign because of: {}", res2);
    }
}