#pragma once

#include "StringTable.h"
#include <FL/Fl_Tile.H>
#include <string>

class Model;
class IChatTabs;
class IChat;
class User;
class Fl_Tile;
class TextDisplay;
class StringTable;

class ServerMessages: public Fl_Tile
{
public:
    ServerMessages(int x, int y, int w, int h,
                   IChatTabs& iChatTabs, IChat & chat, Model & model);
    virtual ~ServerMessages();

    void initTiles();

private:
    IChatTabs & iChatTabs_;
    IChat & iChat_;
    Model & model_;
    TextDisplay * text_;
    StringTable * userList_;

    int handle(int event);
    StringTableRow makeRow(User const & user);
    std::string flagsString(User const & user);

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
};
