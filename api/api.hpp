#pragma once
#include <initializer_list>
#include <Geode/Geode.hpp>

using namespace geode::prelude;

#ifdef GEODE_IS_WINDOWS
#ifdef GDLAPI_EXPORTING
#define GDLAPI_DLL __declspec(dllexport)
#else
#define GDLAPI_DLL __declspec(dllimport)
#endif
#else
#define GDLAPI_DLL __attribute__((visibility("default")))
#endif

namespace gdl {
    enum Language {
        GDL_ENGLISH,
        GDL_RUSSIAN
    };

    GDLAPI_DLL Language getCurrentLanguage();

    GDLAPI_DLL const char* getLanguageName(Language lang = gdl::getCurrentLanguage());
    GDLAPI_DLL const char* getLanguageCodename(Language lang = gdl::getCurrentLanguage());
    GDLAPI_DLL const char* getTranslation(const char* id, Language lang = gdl::getCurrentLanguage());
    GDLAPI_DLL std::unordered_map<std::string, std::unordered_map<Language, std::string>> getApiStrings();
    GDLAPI_DLL std::unordered_map<std::string, matjson::Value> getLocations();
    GDLAPI_DLL bool hasTranslation(const char* id, Language lang = gdl::getCurrentLanguage());
    GDLAPI_DLL void addTranslation(const char* id, Language lang, const char* translatedStr);
    GDLAPI_DLL void addTranslations(const char* id, std::initializer_list<std::pair<Language, const char*>> translations);
    GDLAPI_DLL void addTranslationsFromFile(Language lang, std::filesystem::path pathToJson);
};

GDLAPI_DLL const char* operator""_gdl(const char* str, size_t size);