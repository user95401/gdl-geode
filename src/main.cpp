#include <Geode/Geode.hpp>
#include <Geode/loader/SettingEvent.hpp>
#include "hooks.hpp"

using namespace geode::prelude;

$execute {
	hooks::locationsFile = gdlutils::loadJson((Mod::get()->getResourcesDir() / "ru_ru_locations.json").string());

    // fix crash because of cyrillic Р, credits to sleepyut
    Mod::get()->patch((void*)(base::get() + 0x293658), {0x01});

	hooks::initPatches();
    
    // Change AchievementCell's text width
    float achCellWidth = 230.f;
    Mod::get()->patch((void*)(base::get() + 0x2E6660), ByteVector((uint8_t*)&achCellWidth, (uint8_t*)&achCellWidth + 4));

    gdlutils::achievementsTranslation(Mod::get()->getSettingValue<bool>("achievementsTranslation"));

    listenForSettingChanges("framesTranslation", +[](bool value) {
        gdlutils::reloadAll();
    });

    listenForSettingChanges("achievementsTranslation", +[](bool value) {
        gdlutils::achievementsTranslation(value);
        gdlutils::reloadAll();
    });
}