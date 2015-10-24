// This file is part of flobby (GPL v2 or later), see the LICENSE file

#pragma once

#include "LogFile.h"
#include "model/Model.h"

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

class ServerTab: public Fl_Tile
{
public:
    ServerTab(int x, int y, int w, int h,
                   ITabs & iTabs, Model & model);
    virtual ~ServerTab();

    void initTiles();
    std::string logPath();

private:
    ITabs & iTabs_;
    Model & model_;
    TextDisplay2 * text_;
    ChatInput * input_;
    UserList * userList_;
    LogFile logFile_;

    int handle(int event);
    void onInput(std::string const & text);
    void onComplete(std::string const & text, std::size_t pos, std::pair<std::string, std::size_t>& result);

    void append(std::string const & msg, int interest = 0);

    // model signals
    void connected(bool connected);
    void serverInfo(ServerInfo const & si);
    void loginResult(bool success, std::string const & info);
    void message(std::string const & msg, int interest);
    void userJoined(User const & user);
    void userLeft(User const & user);
    void ring(std::string const & userName);
    void downloadDone(Model::DownloadType downloadType, std::string const & name, bool success);
};
