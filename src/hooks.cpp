#include <utf8.h>
#include "utils.hpp"
#include "menu.hpp"

#include <Geode/Geode.hpp>
#include <Geode/modify/CCLabelBMFont.hpp>
#include <Geode/modify/CCTextureCache.hpp>
#include <Geode/modify/CCNode.hpp>
#include <Geode/modify/CCApplication.hpp>
#include <Geode/modify/OptionsLayer.hpp>
#include <Geode/modify/LoadingLayer.hpp>
#include <Geode/modify/LevelLeaderboard.hpp>
#include <Geode/modify/GauntletSelectLayer.hpp>
#include <Geode/modify/GauntletNode.hpp>
#include <Geode/modify/MenuLayer.hpp>

using namespace geode::prelude;
nlohmann::json locationsFile;
std::map<char const*, char const*> const urls = {
    {"http://robtopgames.com/blog/2017/02/01/geometry-dash-newgrounds", "https://www.gdlocalisation.uk/gd/blog/ru/#newgrounds_start"},
    {"http://www.boomlings.com/files/GJGuide.pdf", "https://www.gdlocalisation.uk/gd/gjguide/ru/gjguide_ru.pdf"},
    {"http://www.robtopgames.com/gd/faq", "https://www.gdlocalisation.uk/gd/blog/ru"}
};

void initPatches() {
    static std::vector<std::string> strings;
    
	locationsFile = gdlutils::loadJson((Mod::get()->getResourcesDir() / "ru_ru_locations.json").string());
    auto langFile = gdlutils::loadJson((Mod::get()->getResourcesDir() / "ru_ru.json").string());

    strings.clear();
    strings.reserve(langFile.size());

#ifdef GEODE_IS_WINDOWS
    auto patchFile = gdlutils::loadJson((Mod::get()->getResourcesDir() / "gdl_patches-windows.json").string());

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
    auto patchFile = gdlutils::loadJson((Mod::get()->getResourcesDir() / "gdl_patches-android.json").string());
    
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
#endif
}

class $modify(MenuLayer){
    bool init(){
        if(!MenuLayer::init())
            return false;

        auto winSize = CCDirector::sharedDirector()->getWinSize();

        CCLabelBMFont* text = CCLabelBMFont::create("GDL v1.2", "goldFont.fnt");
        this->addChild(text);
        text->setScale(0.75f);
        text->setID("gdl-version");
        text->setPosition({winSize.width / 2.f, winSize.height - 14.f});

        return true;
    }
};

// #ifdef GEODE_IS_WINDOWS
// class $modify(TextArea){
//     void setString(gd::string str){
//         auto noTagsStr = coloring::removeTags(str);

//         auto lines = gdlutils::splitByWidth(noTagsStr, this->m_width, this->m_fontFile.c_str());

//         if (lines.size() == 0)
//             return;

//         std::string linesGen(lines.size(), '\n');
//         TextArea::setString(gd::string(linesGen));

//         CCArray* letterArray = CCArray::create();
//         CCARRAY_FOREACH_B_TYPE(this->m_label->getChildren(), lbl, CCLabelBMFont) {
//             lbl->setString(lines[ix].c_str());
//             lbl->setAnchorPoint({this->m_anchorPoint.x, lbl->getAnchorPoint().y});
//             letterArray->addObjectsFromArray(lbl->getChildren());
//         }

//         this->m_label->m_letterArray->removeAllObjects();
//         this->m_label->m_letterArray->addObjectsFromArray(letterArray);

//         if (!this->m_disableColor)
//             coloring::parseTags(str, letterArray);
//     }
// };

// class $modify(AchievementBar){
//     bool init(char const* title, char const* desc, char const* icon, bool quest){
//         if (!AchievementBar::init(title, desc, icon, quest))
//             return false;

//         auto winSize = CCDirector::sharedDirector()->getWinSize();

//         this->m_icon->setPositionX(-110);

//         this->m_achDesc->setAnchorPoint({0.0, this->m_achDesc->getAnchorPoint().y});
//         this->m_achDesc->setPositionX(0);
//         CCARRAY_FOREACH_B_TYPE(this->m_achDesc->getChildren(), lbl, CCLabelBMFont) {
//             lbl->setAnchorPoint({0.0, lbl->getAnchorPoint().y});
//             lbl->setPositionX(this->m_achDesc->convertToNodeSpaceAR({winSize.width / 2 - 75, 0.0}).x);
//         }

//         this->m_achTitle->setPosition({this->m_achDesc->getParent()->convertToNodeSpaceAR({winSize.width / 2 - 75, 0.0}).x, 22});

//         return true;
//     }
// };
// #endif

class $modify(GauntletNode){
    bool init(GJMapPack* mapPack){
        if (!GauntletNode::init(mapPack)) return false;

        auto packID = mapPack->m_packID;
        if (gdlutils::shouldReverseGauntlet(packID)) {
            auto nameLabel = reinterpret_cast<CCLabelBMFont*>(this->getChildren()->objectAtIndex(3));
            auto nameShadow = reinterpret_cast<CCLabelBMFont*>(this->getChildren()->objectAtIndex(5));
            auto gauntletLabel = reinterpret_cast<CCLabelBMFont*>(this->getChildren()->objectAtIndex(4));
            auto gauntletShadow = reinterpret_cast<CCLabelBMFont*>(this->getChildren()->objectAtIndex(6));

            nameLabel->setPositionY(75);
            nameShadow->setPositionY(72);
            gauntletLabel->setPositionY(94);
            gauntletShadow->setPositionY(91);

            nameLabel->setScale(0.45f);
            nameShadow->setScale(0.45f);
            gauntletLabel->setScale(0.62f);
            gauntletShadow->setScale(0.62f);
        }

        return true;
    }
};


// Что это за DooM?????
class $modify(GauntletSelectLayer){
    bool init(int gauntletType){
        if (!GauntletSelectLayer::init(gauntletType))
            return false;

        CCLabelBMFont* nameLabel = nullptr;
        CCLabelBMFont* shadowLabel = nullptr;

        // you cant do it with class members or just getChildren()->objectAtIndex()
        CCARRAY_FOREACH_B_TYPE(this->getChildren(), node, CCNode) {
            if (dynamic_cast<CCLabelBMFont*>(node)) {
                if (nameLabel == nullptr)
                    nameLabel = reinterpret_cast<CCLabelBMFont*>(node);
                else if (shadowLabel == nullptr)
                    shadowLabel = reinterpret_cast<CCLabelBMFont*>(node);
            }
        }

        // ------------------
        // crash start here 
        // -----------------

        // auto gauntletName = gdlutils::splitString(nameLabel->getString(), ' ')[0];
        
        // std::string newName;

        // if(gdlutils::shouldReverseGauntlet(gauntletType)){
        //     newName = fmt::format("Остров {}", gauntletName);
        // }else{
        //     newName = fmt::format("{} Остров", gauntletName);
        // }

        // nameLabel->setString(newName.c_str());
        // shadowLabel->setString(newName.c_str());

        return true;
    }
};

class $modify(LevelLeaderboard){
    bool init(GJGameLevel* lvl, LevelLeaderboardType type, LevelLeaderboardMode mode){
        if (!LevelLeaderboard::init(lvl, type, mode))
            return false;

        CCLabelBMFont* lbl = nullptr;

        CCARRAY_FOREACH_B_TYPE(dynamic_cast<CCNode*>(this->getChildren()->objectAtIndex(0))->getChildren(), node, CCNode) {
            if (dynamic_cast<CCLabelBMFont*>(node)) {
                if (lbl == nullptr)
                    lbl = reinterpret_cast<CCLabelBMFont*>(node);
            }
        }

        lbl->setString(fmt::format("Таблица Лидеров для {}", lvl->m_levelName.c_str()).c_str());

        return true;
    }
};

class $modify(LoadingLayer){
    void loadAssets(){
        if(this->m_loadStep == 10 && Mod::get()->getSettingValue<bool>("framesTranslation")){
            auto plist = (Mod::get()->getResourcesDir() / gdlutils::getQualityString("GDL_TranslatedFrames.plist")).string();
            auto png = (Mod::get()->getResourcesDir() / gdlutils::getQualityString("GDL_TranslatedFrames.png")).string();

            CCSpriteFrameCache::sharedSpriteFrameCache()->removeSpriteFramesFromFile(plist.c_str());

            // Я не понимаю почему функция не может загрузить .png автоматически, ведь в описании указано, что может
            // I can't understand why function can't load .png automatically, the description states that it can
            CCSpriteFrameCache::sharedSpriteFrameCache()->addSpriteFramesWithFile(plist.c_str(),
                CCTextureCache::sharedTextureCache()->addImage(png.c_str(), false)
            );
        }

        LoadingLayer::loadAssets();
    }
};

class $modify(OptionsLayer){
    void customSetup(){
        OptionsLayer::customSetup();

        auto spr = CCSprite::createWithSpriteFrameName("gdlIcon.png"_spr);
        spr->setScale(1.25f);

        auto btn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(GDLMenu::openLayer));
        btn->setPosition({0, 0});

        auto menu = CCMenu::create();
        menu->addChild(btn);
        menu->setPosition({30, 30});
        this->m_mainLayer->addChild(menu, 200);
    }
};

// #ifdef GEODE_IS_ANDROID
// std::string g_currentFont;

// class $modify(MultilineBitmapFont) {
//     bool initWithFont(const char* fontName, gd::string str, float scale, float width, CCPoint anchorPoint, int unk, bool bColourDisabled) {
//         g_currentFont = fontName;
//         if (!MultilineBitmapFont::initWithFont(fontName, str, scale, width, anchorPoint, unk, bColourDisabled))
//             return false;

//         if (!bColourDisabled) {
//             auto letters = cocos2d::CCArray::create();
//             for (int i = 0; i < this->getChildrenCount(); i++) {
//                 auto lbl = (CCNode*)(this->getChildren()->objectAtIndex(i));
//                 letters->addObjectsFromArray(lbl->getChildren());
//             }

//             coloring::parseTags(str, letters);
//         }

//         return true;
//     }

//     gd::string readColorInfo(gd::string str) {
//         return coloring::removeTags(str);
//     }

//     gd::string stringWithMaxWidth(gd::string str, float scaledWidth, float scale) {
//         auto lbl = CCLabelBMFont::create("", g_currentFont.c_str());
//         lbl->setScale(scale);

//         std::string strr = str.c_str();

//         auto hasNL = strr.find("\n") != std::string::npos;
//         auto line = hasNL ? gdlutils::splitString(str, '\n')[0] : strr;

//         float width = scaledWidth / CCDirector::sharedDirector()->getContentScaleFactor();

//         bool overflown = false;
//         std::string current;

//         auto b = line.begin();
//         auto e = line.end();
//         while (b != e) {
//             auto cp = utf8::next(b, e);
//             utf8::append((char32_t)cp, current);

//             lbl->setString(current.c_str());

//             if (lbl->getScaledContentSize().width > width) {
//                 overflown = true;
//                 break;
//             }
//         }

//         if (overflown) {
//             if (current.find(' ') != std::string::npos) {
//                 auto words = gdlutils::splitString(current, ' ');
//                 words.pop_back();
//                 current = gdlutils::joinStrings(words, " ");
//                 current = gdlutils::joinStrings(words, " ") + " ";
//             }
//         } else if (hasNL) {
//             current += " ";
//         }

//         return gd::string(current);
//     }
// };
// #endif

// cocos hooks
class $modify(CCApplication){
    void openURL(char const* url){
        if(urls.contains(url)){
            return CCApplication::openURL(urls.at(url));
        }

        CCApplication::openURL(url);
    }
};

class $modify(CCNode){
    void setPosition(CCPoint p){
        auto lbl = dynamic_cast<CCLabelBMFont*>(this);

        if (!lbl)
            return CCNode::setPosition(p);

        if (locationsFile.contains(lbl->getString())) {
            auto entry = locationsFile[lbl->getString()];
            if (entry.contains("x"))
                p.x += entry["x"].get<float>();
            if (entry.contains("y"))
                p.y += entry["y"].get<float>();
        }

        CCNode::setPosition(p);
    }
};

class $modify(CCLabelBMFont){
    void setScale(float scale){
        if (locationsFile.contains(this->getString())) {
            auto entry = locationsFile[this->getString()];
            if (entry.contains("scale"))
                scale = entry["scale"];
        }

        CCLabelBMFont::setScale(scale);
    }

    static CCLabelBMFont* create(char const* str, char const* fnt){
        if(ghc::filesystem::exists(Mod::get()->getResourcesDir() / gdlutils::getQualityString(fnt))) {
            log::debug("new font {}", (Mod::get()->getResourcesDir() / gdlutils::getQualityString(fnt)).string().c_str());
            return CCLabelBMFont::create(str, (Mod::get()->getResourcesDir() / gdlutils::getQualityString(fnt)).string().c_str());
        }

        return CCLabelBMFont::create(str, fnt);
    }

    bool initWithString(const char* str, const char* fnt, float width, cocos2d::CCTextAlignment align, cocos2d::CCPoint offset){
        log::info("initwithstring {}", fnt);
        auto newFont = Mod::get()->getResourcesDir() / gdlutils::getQualityString(fnt);
        if(ghc::filesystem::exists(newFont)) {
            log::info("new font {}", newFont.string());
            return CCLabelBMFont::initWithString(str, newFont.string().c_str(), width, align, offset);
        }

        return CCLabelBMFont::initWithString(str, fnt, width, align, offset);
    }
};

class $modify(CCTextureCache){
    CCTexture2D* addImage(char const* filename, bool idk){
        auto newPath = Mod::get()->getResourcesDir() / gdlutils::getQualityString(filename);
        if (ghc::filesystem::exists(newPath)) {
            return CCTextureCache::addImage(newPath.string().c_str(), idk);
        }

        return CCTextureCache::addImage(filename, idk);
    }
};

void (__thiscall* std_string_assign_o)(void* self, char* src, size_t len);
void std_string_assign_hk(void* self, char* src, size_t len) {
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
	std_string_assign_o(self, src, strlen(src));
}

$execute {
    initPatches();

	std_string_assign_o = reinterpret_cast<void (__thiscall*)(void* self, char* src, size_t len)>(base::get() + 0x1BB10);
	
	Mod::get()->addHook(
		(void*)(base::get() + 0x1BB10),
		&std_string_assign_hk,
		"gd::string::assign",
		tulip::hook::TulipConvention::Thiscall
	);
}