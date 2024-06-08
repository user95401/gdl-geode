#pragma once
#include <Geode/Geode.hpp>
#include "types.hpp"

using namespace geode::prelude;

class LanguageCell : public geode::GenericListCell {
protected:
    geode::ListView* m_listView = nullptr;
public:
    LanguageCell(LanguageID lang);
    void checkboxClicked(CCObject* sender);
};