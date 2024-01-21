#pragma once
#include "utils.hpp"
#include <Geode/Geode.hpp>
#include <string>
#include <regex>

using namespace geode::prelude;

namespace coloring {
    std::string removeTags(std::string str, bool removeColors = true, bool removeDelay = true, bool removeInstant = true);
    void parseTags(std::string str, CCArray* letters);
} // namespace coloring