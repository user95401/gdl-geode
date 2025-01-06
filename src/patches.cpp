#include <Geode/Geode.hpp>
#include "utils.hpp"
#include "patches.hpp"

using namespace geode::prelude;

#include "api.hpp"

void patches::fixCyrillicP() {
    // this fixes a bug when comments with cyrillic Р (and other letters containing byte 0xA0) are replaced with something by robtop and break the unicode sequence.
    // We patch any other unused byte instead of 0xA0.
#if defined(GEODE_IS_WINDOWS)
    // old address: 0xB44E6
    auto res = Mod::get()->patch((void*)(base::get() + 0xb61d6), {0x48, 0xC7, 0xC0, 0x01, 0x00, 0x00, 0x00}); // mov rax, 0x01
#elif defined(GEODE_IS_ANDROID32)
    // auto res = Mod::get()->patch((void*)(base::get() + 0x93E795), {0x01});
#elif defined(GEODE_IS_ANDROID64)
    // auto res = Mod::get()->patch((void*)(base::get() + 0xE7F440), {0x01});
#else
    #error "Unsupported platform"
#endif

    if (res.isErr())
        log::warn("Failed to patch the Рррр fix ({}), be aware that CommentCell with cyrillic comments may crash!", res.err());
}

void patches::patchStrings() {
    static std::vector<std::string> strings;

    for (auto p : Mod::get()->getPatches()) {
        if (p->isEnabled()) {
            auto res = p->disable().err();
            if (res != std::nullopt) {
                log::warn("Failed to disable patch at {}, error: {}", p->getAddress() - base::get(), res);
            }
        }
    }

    patches::fixCyrillicP();

#ifdef GEODE_IS_WINDOWS64
        bool res3;
        res3 = gdl::patchCString(base::get() + 0x320CB5, "Настройки");
//     log::debug("{}", res3);
//     res3 = gdl::patchCString(base::get() + 0x350598, "Привет, мир 2!");
//     log::debug("{}", res3);
//     res3 = gdl::patchCString(base::get() + 0x3505F1, "Привет, мир 3!");
//     log::debug("{}", res3);

//     bool res2;
//     res2 = gdl::patchStdStringRel("hi", 0x31561F, 0x31562F, 0x315638, {0x315641, 0x315648, 0x31564B, 0x315652, 0x315656, 0x31565C, 0x31565F, 0x315666, 0x31566A});
//     // res2 = gdl::patchStdStringRel("This is a very very long string1!This is a very very long string2!This is a very very long string3!This is a very very long string4!This is a very very long string5!", 0x31561F, 0x31562F, 0x315638, {0x315641, 0x315648, 0x31564B, 0x315652, 0x315656, 0x31565C, 0x31565F, 0x315666, 0x31566A});
//     log::debug("res {}", res2);

    // bool res2;
    // res2 = gdl::patchStdString2("Hello world! This is a long enough string!", {{base::get() + 0x31561F, 0x4F}}, base::get() + 0x31562A); // quit text
    // log::debug("res {}", res2);
    // res2 = gdl::patchStdString2("Test!", {{base::get() + 0x461B64, 0x24}}, base::get() + 0x461B76); // cancel btn in comments posting menu
    // log::debug("res {}", res2);
    // res2 = gdl::patchStdString2("Hello!", {{base::get() + 0x67032, 0x2a}}, base::get() + 0x67052); // "Ease Out" which is inlined
    // log::debug("res {}", res2);
#endif

    auto languageID = gdl::getCurrentLanguage();

    if (languageID == gdl::GDL_ENGLISH) {
        return;
    }

    auto languageCodename = gdl::getLanguageCodename(languageID);
    auto locationsFile = gdlutils::loadJson((Mod::get()->getResourcesDir() / fmt::format("{}-locations.json", languageCodename)).string());
    auto langFile = gdlutils::loadJson((Mod::get()->getResourcesDir() / fmt::format("{}-lang.json", languageCodename)).string());

    if (locationsFile == nullptr || langFile == nullptr) {
        return;
    }

    strings.clear();
    strings.reserve(langFile.size());
    
#if defined(GEODE_IS_WINDOWS)
    // auto patchFile = gdlutils::loadJson((Mod::get()->getResourcesDir() / fmt::format("win-{}.json", Loader::get()->getGameVersion())).string());

    // for (const auto& pair : langFile.items()) {
    //     if (!patchFile.contains(pair.key()))
    //         continue;

    //     strings.push_back(pair.value());

    //     for (const auto addr : patchFile[pair.key()]) {
    //         const char* str = strings[strings.size() - 1].c_str();
    //         auto res = Mod::get()->patch((void*)(base::get() + (uintptr_t)addr), ByteVector((uint8_t*)&str, (uint8_t*)&str + 4)).err();
    //         if (res != std::nullopt) {
    //             log::warn("Failed to patch string at 0x{:X}, error: {}", (uintptr_t)addr, res);
    //         }
    //     }
    // }
#elif defined(GEODE_IS_ANDROID32)
    auto patchFile = gdlutils::loadJson((Mod::get()->getResourcesDir() / "android32-2.205.json").string());

    for (const auto& pair : langFile.items()) {
        if (!patchFile.contains(pair.key()))
            continue;

        strings.push_back(pair.value());

        const char* str = strings[strings.size() - 1].c_str();
        auto array = patchFile[pair.key()].get<nlohmann::json::array_t>();

        for(const auto& addr : array[0].get<nlohmann::json::array_t>()) {
            Mod::get()->patch((void*)(base::get() + addr.get<uintptr_t>()), ByteVector((uint8_t*)&str, (uint8_t*)&str + 4));
        }

        for(const auto& addr : array[1].get<nlohmann::json::array_t>()) {
            Mod::get()->patch((void*)(base::get() + addr.get<uintptr_t>()), ByteVector({0x00, 0xBF}));
        }
    }
#elif defined(GEODE_IS_ANDROID64)
    auto patchFile = gdlutils::loadJson((Mod::get()->getResourcesDir() / "android64-2.205.json").string());

    // coming soon
#endif
}