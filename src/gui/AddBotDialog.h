// This file is part of flobby (GPL v2 or later), see the LICENSE file

#pragma once

#include "model/AI.h"

#include <FL/Fl_Window.H>
#include <vector>
#include <string>

class Model;
class Fl_Hold_Browser;
class Fl_Text_Display;
class Fl_Text_Buffer;
class Fl_Input;
class Fl_Return_Button;

class AddBotDialog: public Fl_Window
{
public:
    AddBotDialog(Model & model);
    virtual ~AddBotDialog();

    void show(std::string const & game, std::string const & botName);

private:
    Model & model_;
    Fl_Hold_Browser * list_;
    Fl_Text_Display * info_;
    Fl_Text_Buffer * buf_;
    Fl_Input * botName_;
    Fl_Return_Button * add_;

    std::vector<AI> ais_;

    static void callbackList(Fl_Widget*, void*);
    static void callbackAdd(Fl_Widget*, void*);

    void onList();
    void onAdd();
};
