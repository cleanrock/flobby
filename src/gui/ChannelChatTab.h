#pragma once

#include "LogFile.h"

#include <FL/Fl_Tile.H>
#include <vector>
#include <string>

class Model;
class ITabs;
class User;
class TextDisplay;
class UserList;
class Fl_Input;

class ChannelChatTab: public Fl_Tile
{
public:
    ChannelChatTab(int x, int y, int w, int h, std::string const & channelName,
                ITabs& iTabs, Model & model);
    virtual ~ChannelChatTab();
    void leave();

private:
    ITabs & iTabs_;
    Model & model_;
    TextDisplay * text_;
    Fl_Input * input_;
    UserList * userList_;
    std::string channelName_;
    LogFile logFile_;

    static void onInput(Fl_Widget * w, void * data);
    int handle(int event);
    void append(std::string const & msg, bool interesting = false);

    // model signals
    void topic(std::string const & channelName, std::string const & author, time_t epochSeconds, std::string const & topic);
    void message(std::string const & channelName, std::string const & message);
    void clients(std::string const & channelName, std::vector<std::string> const & clients);
    void userJoined(std::string const & channelName, std::string const & userName);
    void userLeft(std::string const & channelName, std::string const & userName, std::string const & reason);
    void said(std::string const & channelName, std::string const & userName, std::string const & message);

};
