#pragma once

#include "plugin.h"
#include "game_sa/CPed.h"
#include <map>
#include <string>

class AnimViewer {
public:
    AnimViewer();

private:
    static std::map<unsigned int, std::string> animNamesMap;
};
