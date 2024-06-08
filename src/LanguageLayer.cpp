#include "LanguageLayer.hpp"
#include "LanguageCell.hpp"

LanguageLayer* LanguageLayer::create() {
    LanguageLayer* pRet = new LanguageLayer();
    if (pRet && pRet->init("Language")) {
        pRet->autorelease();
    } else {
        CC_SAFE_DELETE(pRet);
    }
	return pRet;
}

void LanguageLayer::customSetup() {
    auto arr = CCArray::create();
    arr->retain();

    for(auto& language : languages) {
        auto lang = new LanguageCell(language.first);
        arr->addObject(lang);
    }

    auto listView = geode::ListView::create(arr);
    listView->setID("language-list");
    this->m_listLayer->addChild(listView, 6);
}