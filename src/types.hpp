#pragma once
#include <map>

typedef enum LanguageID {
    GDL_ENGLISH,
    GDL_RUSSIAN
} LanguageID;

struct Language {
    char const* displayName;
    char const* codename;
};

inline std::unordered_map<LanguageID, Language> languages = {
    {GDL_ENGLISH, {"English", "en"}},
    {GDL_RUSSIAN, {"Русский", "ru"}}
};