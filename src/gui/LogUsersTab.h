#pragma once

#include "LogFile.h"
#include <FL/Fl_Tile.H>
#include <string>

class Model;
class ITabs;
class User;
class Fl_Tile;
class TextDisplay2;
class ChatInput;
class UserList;
class ServerInfo;

class LogUsersTab: public Fl_Tile
{
public:
    LogUsersTab(int x, int y, int w, int h,
                   ITabs & iTabs, Model & model);
    virtual ~LogUsersTab();

    void initTiles();

private:
    ITabs & iTabs_;
    Model & model_;
    TextDisplay2 * text_;
    ChatInput * input_;
    UserList * userList_;
    LogFile logFile_;

    int handle(int event);
    void onInput(std::string const & text);
    void onComplete(std::string const & text, std::string & result);

    void append(std::string const & msg, bool interesting = false);

    // model signals
    void connected(bool connected);
    void serverInfo(ServerInfo const & si);
    void loginResult(bool success, std::string const & info);
    void message(std::string const & msg);
    void userJoined(User const & user);
    void userLeft(User const & user);
    void ring(std::string const & userName);
    void downloadDone(std::string const & name, bool success);
};
