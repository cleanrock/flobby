// This file is part of flobby (GPL v2 or later), see the LICENSE file

#pragma once

#include <FL/Fl_Window.H>
#include <FL/Fl_Preferences.H>
#include <string>
#include <map>

class Model;
class Fl_Input;
class Fl_Choice;

class OpenBattleZkDialog: public Fl_Window
{
public:
    OpenBattleZkDialog(Model& model);
    virtual ~OpenBattleZkDialog();

    void open();

private:
    Model& model_;
    Fl_Preferences prefs_;

    Fl_Input* title_;
    Fl_Choice* type_;
    Fl_Input* password_;

    std::map<int, int> index2type_;

    void setTitle();

    static void callbackType(Fl_Widget*, void*);
    static void callbackOpen(Fl_Widget*, void*);
};
