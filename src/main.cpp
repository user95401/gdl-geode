#include <Geode/Geode.hpp>
#include <Geode/loader/SettingEvent.hpp>
// #include "hooks.hpp"
// #include <map>

using namespace geode::prelude;

$execute {
// 	hooks::locationsFile = gdlutils::loadJson((Mod::get()->getResourcesDir() / "ru_ru_locations.json").string());

// #ifdef GEODE_IS_WINDOWS
//     // fix crash because of cyrillic Р, credits to sleepyut
//     Mod::get()->patch((void*)(base::get() + 0x293658), {0x01});

//     // Change AchievementCell's text width
//     float achCellWidth = 230.f;
//     Mod::get()->patch((void*)(base::get() + 0x2E6660), ByteVector((uint8_t*)&achCellWidth, (uint8_t*)&achCellWidth + 4));

//     /*
//         0x5547A, 0xCFDDE, 0x137914, 0x1FCC4F - bigFont.fnt
//         0xC9821 - GameManager::getFontFile
//         0x85195 - EditorUI::getCreateBtn
//         0xCFDAA, 0xCFDDE - GameObject::updateTextObject
//         0x17692 - CCCounterLabel::init
        
//         0x55482, 0xCFDE6, 0x13791C, 0x1FCC57 - gjFont%02d.fnt
//         0xC979F, 0xC97CB - GameManager::loadFont
//         0xC982C - GameManager::getFontFile
//         0xCFDE6 - GameObject::updateTextObject
//     */

//     const std::map<std::vector<uintptr_t>, const char*> patchMap = {
//         {{0x5547B, 0xCFDDF, 0x137915, 0x1FCC50, 0xC9822, 0x85196, 0xCFDAB, 0xCFDDF, 0x17693}, "bigFont.fnt"_spr},
//         {{0x55483, 0xCFDE7, 0x13791D, 0x1FCC58, 0xC97A0, 0xC97CC, 0xC982D, 0xCFDE7}, "gjFont%02d.fnt"_spr}
//     };
    
//     for(auto& pair : patchMap) {
//         for (auto& addr : pair.first) {
//             Mod::get()->patch((void*)(base::get() + addr), ByteVector((uint8_t*)&pair.second, (uint8_t*)&pair.second + 4));
//         }
//     }
// #elif defined(GEODE_IS_ANDROID)
//     // fix crash because of cyrillic Р, credits to sleepyut
//     Mod::get()->patch((void*)(base::get() + 0x657473), {0x01});

//     // Change AchievementCell's text width
//     float achCellWidth = 230.f;
//     Mod::get()->patch((void*)(base::get() + 0x226F8C), ByteVector((uint8_t*)&achCellWidth, (uint8_t*)&achCellWidth + 4));

//     // change MapPackCell's View button height
//     auto mapPackCellBtnHeight = 0.f; // 0 => auto detect height
//     Mod::get()->patch((void*)(base::get() + 0x228E3C), ByteVector((uint8_t*)&mapPackCellBtnHeight, (uint8_t*)&mapPackCellBtnHeight + 4));
// #endif

// 	hooks::initPatches();

//     gdlutils::achievementsTranslation(Mod::get()->getSettingValue<bool>("achievementsTranslation"));

//     listenForSettingChanges("framesTranslation", +[](bool value) {
//         gdlutils::reloadAll();
//     });

//     listenForSettingChanges("achievementsTranslation", +[](bool value) {
//         gdlutils::achievementsTranslation(value);
//         auto notification = Notification::create("Restart gd for apply changes", CCSprite::create("GJ_infoIcon_001.png"), 1.0f);
//         notification->show();
//     });
}