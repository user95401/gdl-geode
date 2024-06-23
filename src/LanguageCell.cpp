#include "LanguageCell.hpp"
#include "LanguageLayer.hpp"

LanguageCell::LanguageCell(gdl::Language lang) : geode::GenericListCell(gdl::getLanguageCodename(lang),  {358, 40}) {
    this->setID(fmt::format("lang-{}", gdl::getLanguageCodename(lang)));
    
    auto displayName = CCLabelBMFont::create(gdl::getLanguageName(lang), "bigFont.fnt");
    displayName->setPosition(10, this->m_height / 2);
    displayName->setScale(0.8f);
    displayName->setAnchorPoint({0.0f, 0.5f});

    this->m_mainLayer->addChild(displayName);

    auto toggle = geode::cocos::CCMenuItemExt::createTogglerWithStandardSprites(1.0f, [this, lang](CCMenuItemToggler* sender) {
        if(sender->isToggled()) {
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
    
    toggle->setPosition(menu->convertToNodeSpace({this->m_width - (toggle->getContentWidth() / 2) - 10, m_height / 2}));
    toggle->setID("lang-checkbox");
    
    toggle->toggle(Mod::get()->getSavedValue<int>("language-id") == lang);
    this->m_mainLayer->addChild(menu);
}