#pragma once
#include <Geode/Geode.hpp>
#include <stringPatch.hpp>

using namespace geode::prelude;

class LanguageLayer : public GJDropDownLayer {
protected:
    static inline gdl::Language m_selectedLang;

public:
    static LanguageLayer* create();

    virtual void customSetup();
    void static setSelectedLang(gdl::Language lang);
};