#include "utils.hpp"

#include <Geode/Geode.hpp>

using namespace geode::prelude;

#define CURRENT_LOC "ru"

json gdlutils::loadJson(const std::string& name) {
    std::ifstream translationFileStream(name);

    if (translationFileStream) {
        try {
            nlohmann::json translationObj;
            translationFileStream >> translationObj;

            return translationObj;
        } catch (...) {
            log::error("Failed to parse json \"{}\". Please check the file for mistakes.", name);

            return nullptr;
        }
    } else {
        log::error("Failed to open json \"{}\".", name);

        return nullptr;
    }
}