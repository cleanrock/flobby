#pragma once

#include <FL/Fl_Window.H>
#include <string>

class Fl_Check_Button;
class Fl_File_Input;

class LoggingDialog: public Fl_Window
{
public:
    LoggingDialog();
    virtual ~LoggingDialog();

    void show();

private:
    Fl_Check_Button * logDebug_;
    Fl_File_Input * logFilePath_;
    Fl_Check_Button * logChats_;

    static void callbackApply(Fl_Widget*, void*);
    void badFilePath(std::string const & msg);
};
