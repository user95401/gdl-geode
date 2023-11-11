#include "menu.hpp"

GDLMenu::GDLMenu() : m_page1(nullptr), m_page2(nullptr), m_curPage(1), m_nextBtn(nullptr), m_prevBtn(nullptr) {}

void GDLMenu::switchToPage(size_t page) {
    m_curPage = page;

    switch (page) {
    case 1:
        m_prevBtn->setEnabled(false);
        m_prevBtn->setVisible(false);

        m_nextBtn->setEnabled(true);
        m_nextBtn->setVisible(true);

        m_page1->setVisible(true);
        m_page2->setVisible(false);
        break;
    case 2:
        m_prevBtn->setEnabled(true);
        m_prevBtn->setVisible(true);

        m_nextBtn->setEnabled(false);
        m_nextBtn->setVisible(false);

        m_page1->setVisible(false);
        m_page2->setVisible(true);
        break;
    }
}

void GDLMenu::changePage(CCObject* pObj) {
    switchToPage(m_curPage + (pObj->getTag() ? 1 : -1));
}

void GDLMenu::openLink(CCObject*) {
    CCApplication::sharedApplication()->openURL("https://bit.ly/3K4SbF2");
}

void GDLMenu::setup() {
    CCDirector::sharedDirector()->getMouseDispatcher()->addDelegate(this);
    m_page1 = CCLayer::create();
    m_page2 = CCLayer::create();

    auto dispatcher = CCDirector::sharedDirector()->getTouchDispatcher();
    dispatcher->setForcePrio(true);
    dispatcher->incrementForcePrio(200);
    dispatcher->addTargetedDelegate(this, 200, true);

    // title

    auto title = CCSprite::createWithSpriteFrameName("gdlLogo.png"_spr);
    title->setScale(1.1f);
    auto titlespr = CCMenuItemSprite::create(title, title, this, menu_selector(GDLMenu::openLink));
    titlespr->runAction(CCRepeatForever::create(CCSequence::create(CCEaseInOut::create(CCScaleTo::create(1.5f, 1.2f), 2.0f),
                                                                   CCEaseInOut::create(CCScaleTo::create(1.5f, 1.1f), 2.0f), nullptr)));

    titlespr->setPosition({-10, 84});
    m_buttonMenu->addChild(titlespr);

    // page 1

    auto a1 = CCLabelBMFont::create("Разработчики:", "goldFont.fnt");
    m_page1->addChild(a1);
    a1->setScale(0.8f);
    a1->setPositionY(61);

    auto jaan = gdlutils::createMenuProfile("Jaan", "jaan.png"_spr, false);
    auto mye = gdlutils::createMenuProfile("Mye", "mye.png"_spr, false);
    auto demi = gdlutils::createMenuProfile("Demi Sans", "demisans.png"_spr, true);
    auto lukas = gdlutils::createMenuProfile("RayChan", "lukasradon.png"_spr, true);
    auto gus = gdlutils::createMenuProfile("RNMNX", "guschin.png"_spr, false);

    auto row1 = CCMenu::create(jaan, mye, demi, lukas, gus, nullptr);
    m_page1->addChild(row1);
    row1->alignItemsHorizontallyWithPadding(55);
    row1->setPosition({0, 26});

    auto max = gdlutils::createMenuProfile("Махимал", "maximal.png"_spr, false);
    auto raelc = gdlutils::createMenuProfile("Raelc", "raelc.png"_spr, false);
    auto noname = gdlutils::createMenuProfile("Pixelsuft", "noname.png"_spr, false);
    auto uulti = gdlutils::createMenuProfile("Astragel", "uulti.png"_spr, false);

    auto row2 = CCMenu::create(max, raelc, noname, uulti, nullptr);
    m_page1->addChild(row2);
    row2->alignItemsHorizontallyWithPadding(55);
    row2->setPosition({0, -30});

    m_mainLayer->addChild(m_page1);
    m_page1->setPosition({CCDirector::sharedDirector()->getWinSize().width / 2, 157});

    // page 2

    auto a2 = CCLabelBMFont::create("Отдельная благодарность:", "goldFont.fnt");
    m_page2->addChild(a2);
    a2->setScale(0.7f);
    a2->setPositionY(63);

    const char* thanks1 = "- RobTop\n"
                          "- QuasarKim\n"
                          "- Kolyah35\n"
                          "- Geode SDK\n"
                          "- GD Programming\n"
                          "- Andre\n"
                          "- mat\n"
                          "- HJFod\n"
                          "- Ayrelia";

    const char* thanks2 = "- alk1m123\n"
                          "- Cvolton\n"
                          "- Wylie\n"
                          "- sleepyut\n"
                          "- Log0\n"
                          "- camila314\n"
                          "- Eldyj\n"
                          "- Rektor\n"
                          "- Тестерам GDL";

    auto ta1 = CCLabelBMFont::create(thanks1, "goldFont.fnt");
    ta1->setAnchorPoint({0, 1});
    ta1->setScale(0.6f);
    ta1->setPosition({-130, 50});
    auto ta2 = CCLabelBMFont::create(thanks2, "goldFont.fnt");
    ta2->setAnchorPoint({0, 1});
    ta2->setScale(0.6f);
    ta2->setPosition({40, 50});
    m_page2->addChild(ta1);
    m_page2->addChild(ta2);

    m_page2->setPosition({CCDirector::sharedDirector()->getWinSize().width / 2, 157});
    m_mainLayer->addChild(m_page2);

    // btns

    auto _spr = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
    _spr->setFlipX(true);
    m_nextBtn = CCMenuItemSpriteExtra::create(_spr, this, menu_selector(GDLMenu::changePage));
    m_nextBtn->setTag(1);
    m_nextBtn->setPositionX(225);
    m_buttonMenu->addChild(m_nextBtn);

    m_prevBtn =
        CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png"), this, menu_selector(GDLMenu::changePage));
    m_prevBtn->setTag(0);
    m_prevBtn->setPositionX(-225);
    m_buttonMenu->addChild(m_prevBtn);

    // close btn

    auto closeBtnSpr = ButtonSprite::create("OK", 0, false, "goldFont.fnt", "GJ_button_01.png", 0, 1.);
    auto closeBtn = CCMenuItemSpriteExtra::create(dynamic_cast<CCSprite*>(closeBtnSpr), this, menu_selector(GDLMenu::onClose));
    closeBtn->setUserObject(this);
    closeBtn->setPositionY(-95);
    m_buttonMenu->addChild(closeBtn);

    switchToPage(1);
}

void GDLMenu::keyBackClicked() {
    onClose(nullptr);

    
    CCDirector::sharedDirector()->getMouseDispatcher()->forceRemoveDelegate(this);

    auto dispatcher = CCDirector::sharedDirector()->getTouchDispatcher();
    dispatcher->removeDelegate(this);
    dispatcher->decrementForcePrio(200);
    dispatcher->setForcePrio(false);
}

void GDLMenu::openLayer(CCObject* pObj) {
    CCDirector::sharedDirector()->getRunningScene()->addChild(GDLMenu::create(), 200);
}

GDLMenu* GDLMenu::create() {
    auto pRet = new GDLMenu();

    if (pRet && pRet->init(400, 230)) {
        pRet->autorelease();
        return pRet;
    }

    CC_SAFE_DELETE(pRet);
    return nullptr;
}