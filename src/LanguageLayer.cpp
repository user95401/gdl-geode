#include "LanguageLayer.hpp"
#include "LanguageCell.hpp"
#include <stringPatch.hpp>
#include "patches.hpp"
#include "utils.hpp"

LanguageLayer* LanguageLayer::create() {
    LanguageLayer* pRet = new LanguageLayer();
    if (pRet && pRet->init("language"_gdl)) {
        pRet->autorelease();
    } else {
        CC_SAFE_DELETE(pRet);
    }
	return pRet;
}

void LanguageLayer::customSetup() {
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
        if(Mod::get()->getSavedValue<int>("language-id") == (int)m_selectedLang) {
            return;
        }

        Mod::get()->setSavedValue<int>("language-id", (int)m_selectedLang);
        patches::patchStrings();
        gdlutils::reloadAll();
    });

    applyBtn->setID("apply"_spr);

    auto menu = CCMenu::createWithItem(applyBtn);
    menu->setContentSize(applyBtn->getContentSize());
    menu->setPosition({this->m_mainLayer->getContentWidth() / 2, 35});
    menu->setID("apply-menu"_spr);

    applyBtn->setPosition({0, 0});
    applyBtn->setEnabled(false);
    applyBtn->setVisible(false);
    this->m_mainLayer->addChild(menu);
}

void LanguageLayer::setSelectedLang(gdl::Language lang) {
    m_selectedLang = lang;

    auto applyBtn = (CCMenuItemSpriteExtra*)CCScene::get()->getChildByIDRecursive("apply"_spr);
    applyBtn->setEnabled(Mod::get()->getSavedValue<int>("language-id") != (int)m_selectedLang);
    applyBtn->setVisible(Mod::get()->getSavedValue<int>("language-id") != (int)m_selectedLang);
}