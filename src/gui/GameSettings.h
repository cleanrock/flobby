// This file is part of flobby (GPL v2 or later), see the LICENSE file

#pragma once

#include "StringTable.h"

#include <string>

class Model;

class GameSettings: public StringTable
{
public:
    GameSettings(int x, int y, int w, int h, Model & model);

private:
    Model & model_;

//    StringTableRow makeRow(User const & user);
//    std::string statusString(User const & user);

    // StringTable signals
//    void rowClicked(int rowIndex, int button);
//    void rowDoubleClicked(int rowIndex, int button);

    // model signals
    void setScriptTag(std::string const & key, std::string const & value); // e.g. GAME/StartMetal, 1000
    void removeScriptTag(std::string const & key);
};
