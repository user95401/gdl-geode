#pragma once
#include "json/json.hpp"
#include <string>
#include <vector>
#include <iostream>
#include <Geode/Geode.hpp>

using namespace geode::prelude;

#define CCARRAY_FOREACH_B_BASE(__array__, __obj__, __type__, __index__)                                                                    \
    if (__array__ && __array__->count())                                                                                                   \
        for (auto [__index__, __obj__] = std::tuple<unsigned int, __type__> {0u, nullptr};                                                 \
             (__index__ < __array__->count() && (__obj__ = reinterpret_cast<__type__>(__array__->objectAtIndex(__index__)))); __index__++)

#define CCARRAY_FOREACH_B_TYPE(__array__, __obj__, __type__) CCARRAY_FOREACH_B_BASE(__array__, __obj__, __type__*, ix)

namespace gdlutils {
    std::vector<std::string> splitString(const std::string& str, char separator);

    std::string joinStrings(std::vector<std::string> strings, const char* delim);

    // https://en.wikipedia.org/wiki/UTF-8#Encoding
    size_t sequenceLength(uint8_t byte);

    std::string replaceUnicode(const std::string& str);

    std::vector<std::string> splitByWidth(const std::string& src, float width, const char* fontName);

    nlohmann::json loadJson(const std::string& name);

    bool shouldReverseGauntlet(int id);

    std::string getQualityString(std::string filename);

    CCMenuItem* createMenuProfile(char const* name, char const* spriteFrameName, bool smallNick);
    
    void reloadAll();

    void achievementsTranslation(bool enable);
} // namespace utils