// This file is part of flobby (GPL v2 or later), see the LICENSE file

#pragma once

#include "LogFile.h"
#include "model/Model.h"
#include "ChatInput.h"
#include <FL/Fl_Tile.H>
#include <string>

class Model;
class ITabs;
class User;
class Fl_Tile;
class TextDisplay2;
class UserList;
class ServerInfo;

class ServerTab: public Fl_Tile
{
public:
    ServerTab(int x, int y, int w, int h,
                   ITabs & iTabs, Model & model);
    virtual ~ServerTab();

    void initTiles();
    int getSplitPos();
    void setSplitPos(int x);
    std::string logPath();

private:
    ITabs & iTabs_;
    Model & model_;
    TextDisplay2 * text_;
    ChatInput * input_;
    UserList * userList_;
    LogFile logFile_;

    int handle(int event);
    void resize(int x, int y, int w, int h) override;
    static void callbackSplit(Fl_Widget*, void* data);

    void onInput(std::string const & text);
    void onComplete(std::string const & text, std::size_t pos, std::string const& ignore, CompleteResult& result);

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
