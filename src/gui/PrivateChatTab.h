#pragma once

#include "LogFile.h"
#include <FL/Fl_Group.H>
#include <string>

class Model;
class ITabs;
class User;
class TextDisplay;
class Fl_Input;

class PrivateChatTab: public Fl_Group
{
public:
    PrivateChatTab(int x, int y, int w, int h, std::string const & userName,
                ITabs& iTabs, Model & model);
    virtual ~PrivateChatTab();

private:
    std::string userName_;
    ITabs & iTabs_;
    Model & model_;
    LogFile logFile_;
    TextDisplay * text_;
    Fl_Input * input_;

    int handle(int event);
    void append(std::string const & msg, bool interesting = false);
    static void onInput(Fl_Widget * w, void * data);
    // TODO remove (probably not needed) friend class ChatTabs; // needed when chat is started by the remote user

    // model signal handlers
    void say(std::string const & userName, std::string const & msg); // your confirm msg
    void said(std::string const & userName, std::string const & msg); // msg from other
    void userJoined(User const & user);
    void userLeft(User const & user);
};
