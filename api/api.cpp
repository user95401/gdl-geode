#include <Geode/Geode.hpp>
#include "api.hpp"
#include "utils.hpp"

using namespace geode::prelude;

namespace gdl {
    static std::unordered_map<std::string, std::unordered_map<Language, std::string>> apiStrings;

    Language getCurrentLanguage() {
        return (Language)Mod::get()->getSavedValue<int>("language-id", Language::GDL_RUSSIAN);
    }

    const char* getLanguageName(Language language) {
        switch(language) {
            case GDL_ENGLISH: return "English";
            case GDL_RUSSIAN: return "Русский";
            default:          return "Unknown";
        }
    }

    const char* getLanguageCodename(Language language) {
        switch(language) {
            case GDL_ENGLISH: return "en";
            case GDL_RUSSIAN: return "ru";
        }
    }

    void addTranslation(const char* id, Language language, const char* translatedStr) {
        apiStrings[id][language] = translatedStr;
    }

    const char* getTranslation(const char* id, Language language) {
        if(apiStrings.contains(id) && apiStrings[id].contains(language)) {
            return apiStrings[id][language].c_str();
        } else {
            return id; 
        }
    }

    void addTranslations(const char* id, std::initializer_list<std::pair<Language, const char*>> translations) {
        for(auto& translation : translations) {
            addTranslation(id, translation.first, translation.second);
        }
    }

    void addTranslationsFromFile(Language language, std::filesystem::path pathToJson) {
        auto json = gdlutils::loadJson(pathToJson.string());

        for(auto& translation : json.get<json::object_t>()) {
            addTranslation(translation.first.c_str(), language, translation.second.get<json::string_t>().c_str());
        }
    }
};

const char* operator""_gdl(const char* str, size_t size) {
    return gdl::getTranslation(str, gdl::getCurrentLanguage());
}