#include <Geode/Geode.hpp>
#include <regex>
#include "api.hpp"

using namespace geode::prelude;

namespace gdl {
    static std::unordered_map<std::string, std::unordered_map<Language, std::string>> apiStrings;
    static std::unordered_map<std::string, matjson::Value> apiStrLocations;

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

    const char* getTranslation(const char* id, Language language) {
        if (apiStrings.contains(id) && apiStrings[id].contains(language)) {
            return apiStrings[id][language].c_str();
        }
        else {
            return id;
        }
    }

    std::unordered_map<std::string, std::unordered_map<Language, std::string>>
        getApiStrings() {
        return apiStrings;
    }
    std::unordered_map<std::string, matjson::Value> getLocations() {
        return apiStrLocations;
    }
    
    bool hasTranslation(const char* id, Language lang) {
        return (apiStrings.contains(id) && apiStrings[id].contains(lang));
    }

    void addTranslation(const char* id, Language language, const char* translatedStr) {
        apiStrings[id][language] = translatedStr;
    }

    void addTranslations(const char* id, std::initializer_list<std::pair<Language, const char*>> translations) {
        for(auto& translation : translations) {
            addTranslation(id, translation.first, translation.second);
        }
    }

    void addTranslationsFromFile(Language language, std::filesystem::path path) {
        auto json = file::readJson(path);
        if (json.err()) return log::error(
            "err reading {}, {}", path, json.err().value_or("unhandled")
        );
        for(auto& translation : json.unwrapOrDefault()) {
            addTranslation(
                translation.getKey().value_or("unk").c_str(), 
                language, 
                translation.asString().unwrapOrDefault().c_str()
            );
        }

        auto locfile = string::replace(path.string(), "-lang.json", "-locations.json");
        auto locations = file::readJson(locfile);
        if (locations.err()) return log::error(
            "err reading {}, {}", locfile, json.err().value_or("unhandled")
        );
        for(auto& location : json.unwrapOrDefault()) {
            apiStrLocations[location.getKey().value_or("")] = location;
        }
    }
};

const char* operator""_gdl(const char* str, size_t size) {
    return gdl::getTranslation(str, gdl::getCurrentLanguage());
}