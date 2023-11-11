#include "hooks.hpp"

using namespace geode::prelude;

void hooks::initPatches() {
    static std::vector<std::string> strings;
    
    auto langFile = gdlutils::loadJson((Mod::get()->getResourcesDir() / "ru_ru.json").string());
    auto patchFile = gdlutils::loadJson((Mod::get()->getResourcesDir() / "gdl_patches.json").string());

    strings.clear();
    strings.reserve(langFile.size());
    
    for (const auto& pair : langFile.items()) {
        if (!patchFile.contains(pair.key()))
            continue;
    
        strings.push_back(pair.value());
    
        for (const auto addr : patchFile[pair.key()]) {
            const char* str = strings[strings.size() - 1].c_str();
            Mod::get()->patch((void*)(base::get() + addr), ByteVector((uint8_t*)&str, (uint8_t*)&str + 4));
        }
    }
}

#ifdef GDL_INDEV
class $modify(CCKeyboardDispatcher){
    bool dispatchKeyboardMSG(enumKeyCodes key, bool down){
        if (key == KEY_P && down) {
            hooks::locationsFile = gdlutils::loadJson("ru_ru_locations.json");
            hooks::initPatches();

            return true;
        }

        return CCKeyboardDispatcher::dispatchKeyboardMSG(key, down);
    }
};
#endif

class $modify(MenuLayer){
    bool init(){
        if(!MenuLayer::init())
            return false;

        auto winSize = CCDirector::sharedDirector()->getWinSize();

        CCLabelBMFont* text = CCLabelBMFont::create("GDL v1.1.2", "goldFont.fnt");
        this->addChild(text);
        text->setScale(0.75f);
        text->setPosition({winSize.width / 2.f, winSize.height - 14.f});

        return true;
    }
};

class $modify(GDString){
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
    GDString& winAssign(char const* ptr, size_t size){
        return GDString::winAssign(ptr, strlen(ptr));
    }

    GDString& operatorPlus(char const* ptr, size_t size){
        return GDString::operatorPlus(ptr, strlen(ptr));
    }
};

class $modify(TextArea){
    void setString(gd::string str){
        auto noTagsStr = coloring::removeTags(str);

        auto lines = gdlutils::splitByWidth(noTagsStr, this->m_width, this->m_fontFile.c_str());

        if (lines.size() == 0)
            return;

        std::string linesGen(lines.size(), '\n');
        TextArea::setString(gd::string(linesGen));

        CCArray* letterArray = CCArray::create();
        CCARRAY_FOREACH_B_TYPE(this->m_label->getChildren(), lbl, CCLabelBMFont) {
            lbl->setString(lines[ix].c_str());
            lbl->setAnchorPoint({this->getAnchorPoint().x, lbl->getAnchorPoint().y});
            letterArray->addObjectsFromArray(lbl->getChildren());
        }

        this->m_label->m_letterArray->removeAllObjects();
        this->m_label->m_letterArray->addObjectsFromArray(letterArray);

        if (!this->m_disableColor)
            coloring::parseTags(str, letterArray);
    }
};

class $modify(AchievementBar){
    bool init(char const* title, char const* desc, char const* icon, bool quest){
        if (!AchievementBar::init(title, desc, icon, quest))
            return false;

        auto winSize = CCDirector::sharedDirector()->getWinSize();

        this->m_icon->setPositionX(-110);

        this->m_achDesc->setAnchorPoint({0.0, this->m_achDesc->getAnchorPoint().y});
        this->m_achDesc->setPositionX(0);
        CCARRAY_FOREACH_B_TYPE(this->m_achDesc->getChildren(), lbl, CCLabelBMFont) {
            lbl->setAnchorPoint({0.0, lbl->getAnchorPoint().y});
            lbl->setPositionX(this->m_achDesc->convertToNodeSpaceAR({winSize.width / 2 - 75, 0.0}).x);
        }

        this->m_achTitle->setPosition({this->m_achDesc->getParent()->convertToNodeSpaceAR({winSize.width / 2 - 75, 0.0}).x, 22});

        return true;
    }
};

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

        auto gauntletName = gdlutils::splitString(nameLabel->getString(), ' ')[0];
        
        std::string newName;

        if(gdlutils::shouldReverseGauntlet(gauntletType)){
            newName = fmt::format("Остров {}", gauntletName);
        }else{
            newName = fmt::format("{} Остров", gauntletName);
        }

        nameLabel->setString(newName.c_str());
        shadowLabel->setString(newName.c_str());

        return true;
    }
};

class $modify(LevelLeaderboard){
    bool init(GJGameLevel* lvl, LevelLeaderboardType type){
        if (!LevelLeaderboard::init(lvl, type))
            return false;

        CCLabelBMFont* lbl = nullptr;

        CCARRAY_FOREACH_B_TYPE(dynamic_cast<CCNode*>(this->getChildren()->objectAtIndex(0))->getChildren(), node, CCNode) {
            if (dynamic_cast<CCLabelBMFont*>(node)) {
                if (lbl == nullptr)
                    lbl = reinterpret_cast<CCLabelBMFont*>(node);
            }
        }

        lbl->setString(fmt::format("Таблица Лидеров для {}", lvl->m_levelName).c_str());

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
            // I don't understand why the function can't load .png automatically, the description states that it can
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

// cocos hooks
class $modify(CCApplication){
    void openURL(char const* url){
        if(hooks::urls.contains(url)){
            return CCApplication::openURL(hooks::urls.at(url));
        }

        CCApplication::openURL(url);
    }
};

class $modify(CCNode){
    void setPosition(CCPoint p){
        auto lbl = dynamic_cast<CCLabelBMFont*>(this);

        if (!lbl)
            return CCNode::setPosition(p);

        if (hooks::locationsFile.contains(lbl->getString())) {
            auto entry = hooks::locationsFile[lbl->getString()];
            if (entry.contains("x"))
                p.x += entry["x"];
            if (entry.contains("y"))
                p.y += entry["y"];
        }

        CCNode::setPosition(p);
    }
};

class $modify(CCLabelBMFont){
    void setScale(float scale){
        if (hooks::locationsFile.contains(this->getString())) {
            auto entry = hooks::locationsFile[this->getString()];
            if (entry.contains("scale"))
                scale = entry["scale"];
        }

        CCLabelBMFont::setScale(scale);
    }

    CCLabelBMFont* create(char const* str, char const* fnt){
        if(ghc::filesystem::exists(Mod::get()->getResourcesDir() / gdlutils::getQualityString(fnt))) {
            return CCLabelBMFont::create(str, (Mod::get()->getResourcesDir() / fnt).string().c_str());
        }

        return CCLabelBMFont::create(str, fnt);
    }

    bool initWithString(const char* str, const char* fnt, float width, cocos2d::CCTextAlignment align, cocos2d::CCPoint offset){
        if(ghc::filesystem::exists(Mod::get()->getResourcesDir() / gdlutils::getQualityString(fnt))) {
            return CCLabelBMFont::initWithString(str, (Mod::get()->getResourcesDir() / gdlutils::getQualityString(fnt)).string().c_str(), width, align, offset);
        }

        return CCLabelBMFont::initWithString(str, fnt, width, align, offset);
    }
};

class $modify(CCTextureCache){
    CCTexture2D* addImage(char const* filename, bool idk){
        if(std::find(hooks::fonts.begin(), hooks::fonts.end(), filename) != hooks::fonts.end() && ghc::filesystem::exists(Mod::get()->getResourcesDir() / filename)) {
            return CCTextureCache::addImage((Mod::get()->getResourcesDir() / filename).string().c_str(), idk);
        }

        return CCTextureCache::addImage(filename, idk);
    }
};
