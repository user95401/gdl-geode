#include <Geode/Geode.hpp>
#include <patches.hpp>
#include <stringPatch.hpp>

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

    patches::fixCyrillicP();
    patches::patchStrings();

    gdl::addTranslation("language", "Language", gdl::GDL_ENGLISH);
    gdl::addTranslation("language", "Язык", gdl::GDL_RUSSIAN);

    gdl::addTranslation("apply", "Apply", gdl::GDL_ENGLISH);
    gdl::addTranslation("apply", "Применить", gdl::GDL_RUSSIAN);
};