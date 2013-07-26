#pragma once

#include <FL/Fl_Window.H>
#include <boost/signals2/signal.hpp>
#include <string>

class Fl_Int_Input;
class Fl_Box;

class FontSettingsDialog: public Fl_Window
{
public:
    FontSettingsDialog();
    virtual ~FontSettingsDialog();

    static void setupFont();

    void show();

private:
    Fl_Int_Input * fontSize_;
    Fl_Box * box_;

    static int getFontSize();

    static void callback(Fl_Widget*, void*);
    void onApply();
};
