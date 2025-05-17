#pragma once
#include <Geode/Geode.hpp>
#include "api.hpp"

using namespace geode::prelude;

class LanguageCell : public geode::GenericListCell {
public:
    LanguageCell(gdl::Language lang);
};

class LanguageLayer : public GJDropDownLayer {
protected:
    static inline gdl::Language m_selectedLang;
public:
    static LanguageLayer* create() {
        LanguageLayer* pRet = new LanguageLayer();
        if (pRet && pRet->init("language"_gdl)) {
            pRet->autorelease();
        }
        else {
            CC_SAFE_DELETE(pRet);
        }
        return pRet;
    };
    virtual void customSetup() {
        auto arr = CCArray::create();
        arr->retain();

        auto eng = new LanguageCell(gdl::GDL_ENGLISH);
        arr->addObject(eng);

        auto rus = new LanguageCell(gdl::GDL_RUSSIAN);
        arr->addObject(rus);

        auto listView = geode::ListView::create(arr);
        listView->setID("language-list");
        this->m_listLayer->addChild(listView, 6);

        auto applyBtn = cocos::CCMenuItemExt::createSpriteExtra(ButtonSprite::create("apply"_gdl), [](CCMenuItemSpriteExtra* sender) {
            if (Mod::get()->getSavedValue<int>("language-id") == (int)m_selectedLang) {
                return;
            }

            Mod::get()->setSavedValue<int>("language-id", (int)m_selectedLang);
            GameManager::sharedState()->reloadAll(false, false, true);
            });

        applyBtn->setID("apply"_spr);

        auto menu = CCMenu::createWithItem(applyBtn);
        menu->setContentSize(applyBtn->getContentSize());
        menu->setPosition({ this->m_mainLayer->getContentWidth() / 2, 35 });
        menu->setID("apply-menu"_spr);

        applyBtn->setPosition({ 0, 0 });
        applyBtn->setEnabled(false);
        applyBtn->setVisible(false);
        this->m_mainLayer->addChild(menu);
    };
    void static setSelectedLang(gdl::Language lang) {
        m_selectedLang = lang;

        auto applyBtn = (CCMenuItemSpriteExtra*)CCScene::get()->getChildByIDRecursive("apply"_spr);
        applyBtn->setEnabled(Mod::get()->getSavedValue<int>("language-id") != (int)m_selectedLang);
        applyBtn->setVisible(Mod::get()->getSavedValue<int>("language-id") != (int)m_selectedLang);
    };
};

inline LanguageCell::LanguageCell(gdl::Language lang) : geode::GenericListCell(gdl::getLanguageCodename(lang), { 358, 40 }) {
    this->setID(fmt::format("lang-{}", gdl::getLanguageCodename(lang)));

    auto displayName = CCLabelBMFont::create(gdl::getLanguageName(lang), "bigFont.fnt");
    displayName->setPosition(10, this->m_height / 2);
    displayName->setScale(0.8f);
    displayName->setAnchorPoint({ 0.0f, 0.5f });

    this->m_mainLayer->addChild(displayName);

    auto toggle = geode::cocos::CCMenuItemExt::createTogglerWithStandardSprites(1.0f, [this, lang](CCMenuItemToggler* sender) {
        if (sender->isToggled()) {
            sender->setClickable(false);
            return;
        }

        auto listView = (BoomListView*)CCScene::get()->getChildByIDRecursive("language-list");
        auto arr = listView->m_entries;

        CCObject* obj;
        CCARRAY_FOREACH(arr, obj) {
            auto cell = (LanguageCell*)obj;
            auto cellCheckbox = (CCMenuItemToggler*)cell->getChildByIDRecursive("lang-checkbox");
            bool shouldOff = cell->getID() != sender->getID();

            cellCheckbox->toggle(!shouldOff);
            cellCheckbox->setClickable(shouldOff);
        }

        LanguageLayer::setSelectedLang(lang);
        });

    auto menu = CCMenu::createWithItem(toggle);

    toggle->setPosition(menu->convertToNodeSpace({ this->m_width - (toggle->getContentWidth() / 2) - 10, m_height / 2 }));
    toggle->setID("lang-checkbox");

    toggle->toggle(Mod::get()->getSavedValue<int>("language-id") == lang);
    this->m_mainLayer->addChild(menu);
};