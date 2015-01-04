// This file is part of flobby (GPL v2 or later), see the LICENSE file

#pragma once

#include <FL/Fl_Window.H>
#include <string>

class Fl_Check_Button;
class Fl_Output;

class LoggingDialog: public Fl_Window
{
public:
    LoggingDialog();
    virtual ~LoggingDialog();

    void show();

private:
    Fl_Output* flobbyLogPath_;
    Fl_Check_Button* logDebug_;

    Fl_Output* chatLogDir_;
    Fl_Check_Button* logChats_;

    static void callbackApply(Fl_Widget*, void*);
};
