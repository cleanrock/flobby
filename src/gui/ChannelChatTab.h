// This file is part of flobby (GPL v2 or later), see the LICENSE file

#pragma once

#include "LogFile.h"

#include <FL/Fl_Tile.H>
#include <vector>
#include <string>

class Model;
class ITabs;
class User;
class TextDisplay2;
class UserList;
class ChatInput;
class ChatSettingsDialog;

class ChannelChatTab: public Fl_Tile
{
public:
    ChannelChatTab(int x, int y, int w, int h, std::string const & channelName,
                ITabs& iTabs, Model & model, ChatSettingsDialog & chatSettingsDialog);
    virtual ~ChannelChatTab();
    void leave();
    void append(std::string const & msg, int interest = -1);

private:
    ITabs & iTabs_;
    Model & model_;
    ChatSettingsDialog & chatSettingsDialog_;
    TextDisplay2 * text_;
    ChatInput * input_;
    UserList * userList_;
    std::string channelName_;
    LogFile logFile_;
    bool showJoinLeave_;
    bool beep_;

    void onInput(std::string const & text);
    void onComplete(std::string const& text, std::size_t pos, std::pair<std::string, std::size_t>& result);
    int handle(int event);

    void initChatSettings();

    // model signals
    void topic(std::string const & channelName, std::string const & author, time_t epochSeconds, std::string const & topic);
    void message(std::string const & channelName, std::string const & message);
    void clients(std::string const & channelName, std::vector<std::string> const & clients);
    void userJoined(std::string const & channelName, std::string const & userName);
    void userLeft(std::string const & channelName, std::string const & userName, std::string const & reason);
    void said(std::string const & channelName, std::string const & userName, std::string const & message);

};
