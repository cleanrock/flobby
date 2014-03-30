// This file is part of flobby (GPL v2 or later), see the LICENSE file

#pragma once

#include "StringTable.h"
#include <FL/Fl_Group.H>
#include <string>


class User;
class Battle;
class Model;
class Cache;
class Fl_Box;
class Fl_Group;
class Fl_Button;
class Fl_RGB_Image;
class Fl_Multiline_Output;

class BattleInfo: public Fl_Group
{
public:
    BattleInfo(int x, int y, int w, int h, Model & model, Cache & cache);
    virtual ~BattleInfo();

    void battle(Battle const & battle); // call to show a new battle
    void reset();
    int battleId() const;
    void refresh();

private:
    Model & model_;
    Cache & cache_;
    int battleId_;
    Fl_Group *header_;
    Fl_Box *mapImageBox_;
    std::unique_ptr<Fl_RGB_Image> mapImage_;
    std::string currentMapImage_; // optimization, indicates what map image is currently shown to avoid setting the same image
    Fl_Multiline_Output *headerText_;
    StringTable *userList_;

    void setMapImage(Battle const & battle);
    void setHeaderText(Battle const & battle);
    static void onJoin(Fl_Widget* w, void* data);

    // model signal handlers
    void battleChanged(const Battle & battle);
    void battleClosed(Battle const & battle);
    void userJoinedBattle(User const & user, const Battle & battle);
    void userLeftBattle(User const & user, const Battle & battle);
    void userChanged(User const & user);

};

inline int BattleInfo::battleId() const
{
    return battleId_;
}
