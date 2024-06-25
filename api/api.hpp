#pragma once
#include "stringPatch.hpp"
#include <initializer_list>
#include <Geode/Geode.hpp>

using namespace geode::prelude;

namespace gdl {
    enum Language {
        GDL_ENGLISH,
        GDL_RUSSIAN
    };

    GDLAPI_DLL Language getCurrentLanguage();

    GDLAPI_DLL const char* getLanguageName(Language language);
    GDLAPI_DLL const char* getLanguageCodename(Language language);

    GDLAPI_DLL void addTranslation(const char* id, Language language, const char* translatedStr);
    GDLAPI_DLL void addTranslations(const char* id, std::initializer_list<std::pair<Language, const char*>> translations);
    GDLAPI_DLL void addTranslationsFromFile(Language language, std::filesystem::path pathToJson);
};

GDLAPI_DLL const char* operator""_gdl(const char* str, size_t size);