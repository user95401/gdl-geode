#include <Geode/Geode.hpp>
#include <Geode/modify/OptionsLayer.hpp>
#include "LanguageLayer.hpp"

using namespace geode::prelude;

class $modify(GDLOptionsLayer, OptionsLayer) {
    struct Fields {
        bool m_clickedLanguage = false;
    };

    void onLanguage(CCObject* sender) {
        m_fields->m_clickedLanguage = true;
        this->hideLayer(false);
    }

    virtual void customSetup() {
        OptionsLayer::customSetup();

        auto buttonsMenu = (CCMenu*)this->m_mainLayer->getChildren()->objectAtIndex(4);
        if(!buttonsMenu)
            return;

        auto languageBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("language"_gdl, "goldFont.fnt", "GJ_button_01.png"), this, SEL_MenuHandler(&GDLOptionsLayer::onLanguage));
        languageBtn->setPosition(buttonsMenu->getContentWidth() / 2, -115);
        buttonsMenu->addChild(languageBtn);
    }

    virtual void layerHidden() {
        if(m_clickedLanguage) {
            auto languageLayer = LanguageLayer::create();
            this->getParent()->addChild(languageLayer);
            languageLayer->setZOrder(100);
            languageLayer->showLayer(false);
            m_clickedLanguage = false;
        }

        OptionsLayer::layerHidden();
    }
};
