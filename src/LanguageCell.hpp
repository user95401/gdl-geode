#pragma once
#include <Geode/Geode.hpp>
#include <stringPatch.hpp>
using namespace geode::prelude;

class LanguageCell : public geode::GenericListCell {
public:
    LanguageCell(gdl::Language lang);
};