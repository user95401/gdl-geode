#include "Geode/Geode.hpp"
#include "Geode/modify/MultilineBitmapFont.hpp"
#include "Geode/modify/CCLabelBMFont.hpp"
#include "Geode/modify/CCTextureCache.hpp"

#include "utils.hpp"
#include <utf8.h>

using namespace geode::prelude;

class $modify(MultilineBitmapFont) {
    gd::string stringWithMaxWidth(gd::string p0, float p1, float p2) {
        log::debug("{};{};{}", p0, p1, p2);

        std::string str = p0;

        std::string s;
        auto it = str.begin();
        for (auto i = 0u; i < 10 && it != str.end(); i++) {
            auto cp = utf8::next(it, str.end());
            utf8::append(cp, s);
        }
        
        return s;
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
    if (res1 != std::nullopt) {
        log::error("Failed to hook gd::string::assign because of: {}", res1);
    }
}