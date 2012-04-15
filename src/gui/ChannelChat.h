#pragma once

#include "StringTable.h"
#include <FL/Fl_Tile.H>
#include <vector>
#include <string>

class Model;
class User;
class TextDisplay;
class StringTable;
class Fl_Input;

class ChannelChat: public Fl_Tile
{
public:
    ChannelChat(int x, int y, int w, int h, std::string const & channelName, Model & model);
    virtual ~ChannelChat();
    void leave();

//    void initTiles();

private:
    Model & model_;
    TextDisplay * text_;
    Fl_Input * input_;
    StringTable * userList_;

    std::string channelName_;

    int handle(int event);

    static void onInput(Fl_Widget * w, void * data);

    StringTableRow makeRow(std::string const & userName);
    std::string flagsString(User const & user);

    // model signals
    void topic(std::string const & channelName, std::string const & author, time_t epochSeconds, std::string const & topic);
    void clients(std::string const & channelName, std::vector<std::string> const & clients);
    void userJoined(std::string const & channelName, std::string const & userName);
    void userLeft(std::string const & channelName, std::string const & userName, std::string const & reason);
    void said(std::string const & channelName, std::string const & userName, std::string const & message);

};
