#include <Geode/Geode.hpp>

#include "utils.hpp"

using namespace geode::prelude;

$execute {
    SetConsoleOutputCP(65001);
    
    static std::vector<std::string> strings;
    
	// locationsFile = gdlutils::loadJson((Mod::get()->getResourcesDir() / "ru_ru_locations.json").string());
    auto langFile = gdlutils::loadJson((Mod::get()->getResourcesDir() / "ru_lang.json").string());

    strings.clear();
    strings.reserve(langFile.size());

#ifdef GEODE_IS_WINDOWS
    auto patchFile = gdlutils::loadJson((Mod::get()->getResourcesDir() / "windows_patches.json").string());

    for (const auto& pair : langFile.items()) {
        if (!patchFile.contains(pair.key()))
            continue;
    
        strings.push_back(pair.value());
    
        for (const auto addr : patchFile[pair.key()]) {
            const char* str = strings[strings.size() - 1].c_str();
            Mod::get()->patch((void*)(base::get() + addr), ByteVector((uint8_t*)&str, (uint8_t*)&str + 4));
        }
    }
#elif defined(GEODE_IS_ANDROID)
    // auto patchFile = gdlutils::loadJson((Mod::get()->getResourcesDir() / "gdl_patches-android.json").string());
    
    // for (const auto& pair : langFile.items()) {
    //     if (!patchFile.contains(pair.key()))
    //         continue;
    
    //     strings.push_back(pair.value());

    //     const char* str = strings[strings.size() - 1].c_str();
    //     auto array = patchFile[pair.key()].get<nlohmann::json::array_t>();
 
    //     for(const auto& addr : array[0].get<nlohmann::json::array_t>()) {
    //         Mod::get()->patch((void*)(base::get() + addr.get<uintptr_t>()), ByteVector((uint8_t*)&str, (uint8_t*)&str + 4));
    //     }

    //     for(const auto& addr : array[1].get<nlohmann::json::array_t>()) {
    //         Mod::get()->patch((void*)(base::get() + addr.get<uintptr_t>()), ByteVector({0x00, 0xBF}));
    //     }
    // }
#endif
}