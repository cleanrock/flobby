#pragma once

#include "StringTable.h"

#include <FL/Fl_Group.H>
#include <memory>

class Model;
class User;
class Battle;
class BattleInfo;
class Cache;
class BattleFilterDialog;

class BattleList: public Fl_Group
{
public:
    BattleList(int x, int y, int w, int h, Model & model, Cache & cache);
    virtual ~BattleList();

    void showFilterDialog();

    BattleInfo & getBattleInfo();
    void refresh();

private:
    Model & model_;
    StringTable * battleList_;
    BattleInfo * battleInfo_;
    std::string filterGame_;
    int filterPlayers_;
    BattleFilterDialog * battleFilterDialog_;

    // model signal handlers
    //
    void connected(bool connected);
    void loginResult(bool success, std::string const & info);
    void battleOpened(Battle const & battle);
    void battleChanged(Battle const & battle);
    void battleClosed(Battle const & battle);
    void userJoinedLeftBattle(User const & user, const Battle & battle);

    void battleListRowChanged(int rowIndex);
    void battleListRowClicked(int rowIndex, int button);
    void battleListRowDoubleClicked(int rowIndex, int button);

    StringTableRow makeRow(Battle const & battle);
    std::string flagsString(Battle const & battle);

    void joinBattle(Battle const & battle);

    bool passesFilter(Battle const & battle);
    void setFilter(std::string const & game, int players);

};

inline BattleInfo & BattleList::getBattleInfo()
{
    return *battleInfo_;
}
