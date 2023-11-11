#pragma once

#include <Geode/Geode.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/GDString.hpp>
#include <Geode/modify/TextArea.hpp>
#include <Geode/modify/CCApplication.hpp>
#include <Geode/modify/AchievementBar.hpp>
#include <Geode/modify/GauntletNode.hpp>
#include <Geode/modify/CCNode.hpp>
#include <Geode/modify/CCLabelBMFont.hpp>
#include <Geode/modify/GauntletSelectLayer.hpp>
#include <Geode/modify/LevelLeaderboard.hpp>
#include <Geode/modify/LoadingLayer.hpp>
#include <Geode/modify/OptionsLayer.hpp>
#include <Geode/modify/CCTextureCache.hpp>
#include <Geode/modify/CCSpriteFrameCache.hpp>

#include <map>
#include <algorithm>
#include "json.hpp"
#include "utils.hpp"
#include "coloring.hpp"
#include "menu.hpp"

namespace hooks {
    inline nlohmann::json locationsFile;
    inline std::map<char const*, char const*> const urls = {
        {"http://robtopgames.com/blog/2017/02/01/geometry-dash-newgrounds", "https://www.gdlocalisation.uk/gd/blog/ru/#newgrounds_start"},
        {"http://www.boomlings.com/files/GJGuide.pdf", "https://www.gdlocalisation.uk/gd/gjguide/ru/gjguide_ru.pdf"},
        {"http://www.robtopgames.com/gd/faq", "https://www.gdlocalisation.uk/gd/blog/ru"}
    };
    inline std::vector<std::string> fonts = {
        "bigFont.png",  "bigFont-hd.png",  "bigFont-uhd.png", 
        "chatFont.png", "chatFont-hd.png", "chatFont-uhd.png", 
        "goldFont.png", "goldFont-hd.png", "goldFont-uhd.png", 
        "gjFont01.png", "gjFont01-hd.png", "gjFont01-uhd.png", 
        "gjFont02.png", "gjFont02-hd.png", "gjFont02-uhd.png", 
        "gjFont03.png", "gjFont03-hd.png", "gjFont03-uhd.png", 
        "gjFont04.png", "gjFont04-hd.png", "gjFont04-uhd.png", 
        "gjFont05.png", "gjFont05-hd.png", "gjFont05-uhd.png", 
        "gjFont06.png", "gjFont06-hd.png", "gjFont06-uhd.png", 
        "gjFont07.png", "gjFont07-hd.png", "gjFont07-uhd.png", 
        "gjFont08.png", "gjFont08-hd.png", "gjFont08-uhd.png", 
        "gjFont09.png", "gjFont09-hd.png", "gjFont09-uhd.png", 
        "gjFont10.png", "gjFont10-hd.png", "gjFont10-uhd.png", 
        "gjFont11.png", "gjFont11-hd.png", "gjFont11-uhd.png", 
    };

    void initPatches();
};