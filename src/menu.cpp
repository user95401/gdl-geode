#include <Geode/Geode.hpp>
#include <Geode/modify/OptionsLayer.hpp>
#include "LanguageLayer.hpp"

using namespace geode::prelude;

class $modify(GDLOptionsLayer, OptionsLayer) {
    bool m_clickedLanguage = false;

    void onLanguage(CCObject* sender) {
        m_clickedLanguage = true;
        this->hideLayer(false);
    }

    // virtual void customSetup() {
    //     OptionsLayer::customSetup();

    //     auto buttonsMenu = (CCMenu*)this->m_mainLayer->getChildren()->objectAtIndex(4);
    //     if(!buttonsMenu)
    //         return;

    //     log::info("{}", buttonsMenu->getChildrenCount());

    //     // auto accountBtn = (CCMenuItemSpriteExtra*)buttonsMenu->getChildren()->objectAtIndex(0);
    //     // auto tutorialBtn = (CCMenuItemSpriteExtra*)buttonsMenu->getChildren()->objectAtIndex(1);
    //     // auto rateBtn = (CCMenuItemSpriteExtra*)buttonsMenu->getChildren()->objectAtIndex(4);
    //     // auto soundtracksBtn = (CCMenuItemSpriteExtra*)buttonsMenu->getChildren()->objectAtIndex(5);
    //     // auto helpBtn = (CCMenuItemSpriteExtra*)buttonsMenu->getChildren()->objectAtIndex(6);

    //     // accountBtn->setPositionX(rateBtn->getPositionX());
    //     // accountBtn->setContentWidth(rateBtn->getContentWidth());

    //     // tutorialBtn->setPositionX(helpBtn->getPositionX());
    //     // tutorialBtn->setContentWidth(helpBtn->getContentWidth());

    //     auto languageBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("Language", "goldFont.fnt", "GJ_button_01.png"), this, SEL_MenuHandler(&GDLOptionsLayer::onLanguage));
    //     languageBtn->setPosition(0, -130);
    //     buttonsMenu->addChild(languageBtn);
    // }

    // virtual void layerHidden() {
    //     if(m_clickedLanguage) {
    //         auto languageLayer = LanguageLayer::create();
    //         this->getParent()->addChild(languageLayer);
    //         languageLayer->setZOrder(100);
    //         languageLayer->showLayer(false);
    //         m_clickedLanguage = false;
    //     }

    //     OptionsLayer::layerHidden();
    // }
};
