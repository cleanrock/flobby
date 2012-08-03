#pragma once

#include "LogFile.h"

#include <FL/Fl_Group.H>
#include <string>

class Model;
class ITabs;
class User;
class TextDisplay2;
class ChatInput;
class ChatSettingsDialog;

class PrivateChatTab: public Fl_Group
{
public:
    PrivateChatTab(int x, int y, int w, int h, std::string const & userName,
                ITabs& iTabs, Model & model, ChatSettingsDialog & chatSettingsDialog);
    virtual ~PrivateChatTab();

private:
    std::string userName_;
    ITabs & iTabs_;
    Model & model_;
    ChatSettingsDialog & chatSettingsDialog_;
    LogFile logFile_;
    TextDisplay2 * text_;
    ChatInput * input_;
    bool beep_;

    int handle(int event);
    void append(std::string const & msg, bool interesting = false);
    void onInput(std::string const & text);

    void initChatSettings();

    // model signal handlers
    void say(std::string const & userName, std::string const & msg); // your confirm msg
    void said(std::string const & userName, std::string const & msg); // msg from other
    void userJoined(User const & user);
    void userLeft(User const & user);
};
