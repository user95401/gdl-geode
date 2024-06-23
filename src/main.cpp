#include <Geode/Geode.hpp>
#include <patches.hpp>
#include "api.hpp"

using namespace geode::prelude;

$execute {
#if defined(GEODE_IS_WINDOWS)
    SetConsoleOutputCP(65001); // utf8

    // if(Loader::get()->isModLoaded("geode.custom-keybinds")) {
    //     BindManager::get()->registerBindable({
    //         "reload"_spr,
    //         "Reload lang file",
    //         "",
    //         { Keybind::create(KEY_P, Modifier::None) },
    //         "GDL/Debug"
    //     });

    //     new EventListener([=](InvokeBindEvent* event) {
    //         static bool wasPressed = false;

    //         if (event->isDown()) {
    //             if (!wasPressed) {
    //                 wasPressed = true;
    //                 patchStrings();
    //                 Notification::create("GDL: Updated strings", NotificationIcon::Success)->show();
    //             }
    //         } else {
    //             wasPressed = false;
    //         }
            
    //         return ListenerResult::Propagate;
    //     }, InvokeBindFilter(nullptr, "reload"_spr));
    // }
#endif
    patches::patchStrings();

    gdl::addTranslations("language", {{gdl::GDL_ENGLISH, "Language"}, {gdl::GDL_RUSSIAN, "Язык"}});
    gdl::addTranslations("apply", {{gdl::GDL_ENGLISH, "Apply"}, {gdl::GDL_RUSSIAN, "Применить"}});
};