#pragma once
#include "utils.hpp"
// #include "hooks.hpp"
#include "BrownAlertDelegate.hpp"

class GDLMenu : public BrownAlertDelegate {
private:
    CCLayer* m_page1;
    CCLayer* m_page2;
    size_t m_curPage;
    CCMenuItemSpriteExtra* m_nextBtn;
    CCMenuItemSpriteExtra* m_prevBtn;

    GDLMenu();
    void switchToPage(size_t page);
    void keyBackClicked();
    void setup();
    void openLink(CCObject*);
    void changePage(CCObject* pObj);

public:
    static GDLMenu* create();
    void openLayer(CCObject*);
};