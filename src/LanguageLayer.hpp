#pragma once
#include <Geode/Geode.hpp>

using namespace geode::prelude;

class LanguageLayer : public GJDropDownLayer {
public:
    static LanguageLayer* create();

    virtual void customSetup();
};