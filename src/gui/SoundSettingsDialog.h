#pragma once

#include <FL/Fl_Window.H>

class Fl_Check_Button;
class Fl_Input;

class SoundSettingsDialog: public Fl_Window
{
public:
    SoundSettingsDialog();
    virtual ~SoundSettingsDialog();

    void show();

private:
    Fl_Check_Button * enable_;
    Fl_Input * command_;

    static void callbackTest(Fl_Widget*, void*);
    static void callbackApply(Fl_Widget*, void*);

    void loadPrefs();
    void savePrefs();
};
