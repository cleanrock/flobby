#pragma once

#include "StringTable.h"
#include "LogFile.h"
#include <FL/Fl_Tile.H>
#include <string>

class Model;
class ITabs;
class IChat;
class User;
class Fl_Tile;
class TextDisplay;
class StringTable;

class LogUsersTab: public Fl_Tile
{
public:
    LogUsersTab(int x, int y, int w, int h,
                   ITabs& iChatTabs, IChat & chat, Model & model);
    virtual ~LogUsersTab();

    void initTiles();

private:
    ITabs & iTabs_;
    IChat & iChat_;
    Model & model_;
    TextDisplay * text_;
    StringTable * userList_;
    LogFile logFile_;

    int handle(int event);
    StringTableRow makeRow(User const & user);
    std::string statusString(User const & user);

    void append(std::string const & msg, bool interesting = false);
    void userClicked(int rowIndex, int button);
    void userDoubleClicked(int rowIndex, int button);

    // model signals
    void connected(bool connected);
    void loginResult(bool success, std::string const & info);
    void message(std::string const & msg);
    void userJoined(User const & user);
    void userChanged(User const & user);
    void userLeft(User const & user);
    void ring(std::string const & userName);
    void downloadDone(std::string const & name);
};
