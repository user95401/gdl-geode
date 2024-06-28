#pragma once
#include <Geode/Geode.hpp>
#include "api.hpp"

using namespace geode::prelude;

class LanguageCell : public geode::GenericListCell {
public:
    LanguageCell(gdl::Language lang);
};