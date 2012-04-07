#pragma once

#include <FL/Fl_Window.H>
#include <string>

class Model;
class Fl_Hold_Browser;
class Fl_Input;

class AddBotDialog: public Fl_Window
{
public:
    AddBotDialog(Model & model);
    virtual ~AddBotDialog();

    void show(std::string const & game);

private:
    Model & model_;
    Fl_Hold_Browser * list_;
    Fl_Input * botName_;

    static void callbackList(Fl_Widget*, void*);
    static void callbackButton(Fl_Widget*, void*);
};
