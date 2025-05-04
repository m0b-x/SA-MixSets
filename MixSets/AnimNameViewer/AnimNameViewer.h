#pragma once

#include "plugin.h"
#include "game_sa/CPed.h"
#include <unordered_map>
#include <string>

class AnimNameViewer {
public:
    static AnimNameViewer& GetInstance();

private:
    AnimNameViewer();
    AnimNameViewer(const AnimNameViewer&) = delete;
    AnimNameViewer& operator=(const AnimNameViewer&) = delete;

    static std::unordered_map<unsigned int, std::string> animNamesMap;
};
