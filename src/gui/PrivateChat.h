#pragma once

#include <FL/Fl_Group.H>
#include <string>

class Model;
class TextDisplay;
class Fl_Input;

class PrivateChat: public Fl_Group
{
public:
    PrivateChat(int x, int y, int w, int h, std::string const & userName, Model & model);
    virtual ~PrivateChat();

private:
    std::string userName_;
    Model & model_;
    TextDisplay * text_;
    Fl_Input * input_;

    int handle(int event);

    void say(std::string const & userName, std::string const & msg); // your confirm msg
    void said(std::string const & userName, std::string const & msg); // msg from other

    static void onInput(Fl_Widget * w, void * data);

    friend class ChatTabs; // needed when chat is started by the remote user
};
