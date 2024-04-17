#include "LanguageCell.hpp"

LanguageCell::LanguageCell(LanguageID lang) : geode::GenericListCell(languages[lang].codename,  {358, 40}) {
    this->setID(fmt::format("lang-{}", languages[lang].codename));
    
    auto displayName = CCLabelBMFont::create(languages[lang].displayName, "bigFont.fnt");
    displayName->setPosition(10, this->m_height / 2);
    displayName->setScale(0.8f);
    displayName->setAnchorPoint({0.0f, 0.5f});

    this->m_mainLayer->addChild(displayName);

    auto checkbox = CCMenuItemToggler::create(
        CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png"), 
        CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png"),
        this,
        SEL_MenuHandler(&LanguageCell::checkboxClicked)
    );

    auto menu = CCMenu::createWithItem(checkbox);
    
    checkbox->setPosition(menu->convertToNodeSpace({this->m_width - (checkbox->getContentWidth() / 2) - 10, m_height / 2}));
    checkbox->setID("lang-checkbox");
    this->m_mainLayer->addChild(menu);
}

void LanguageCell::checkboxClicked(CCObject* sender) {
    log::info("Changed lang");
    auto checkbox = (CCMenuItemToggler*)sender;

    if(!checkbox->isEnabled()) {
        checkbox->setEnabled(true);
        return;
    }

    auto arr = this->m_tableView->m_cellArray;
    if(!arr) {
        log::info("fck urself");
        return;
    }

    CCObject* obj;
    // CCARRAY_FOREACH(arr, obj) {
    //     auto cell = (geode::GenericListCell*)obj;
    //     if(cell->getID() != checkbox->getID()) {
    //         auto cellCheckbox = (CCMenuItemToggler*)cell->getChildByIDRecursive("lang-checkbox");
    //         cellCheckbox->setEnabled(false);
    //     }
    // }
}