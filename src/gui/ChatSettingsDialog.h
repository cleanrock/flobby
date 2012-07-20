#pragma once

#include <FL/Fl_Window.H>
#include <string>

class Fl_Check_Button;

class ChatSettingsDialog: public Fl_Window
{
public:
    ChatSettingsDialog();
    virtual ~ChatSettingsDialog();

    void show();

private:
    Fl_Check_Button * showJoinLeaveInChannels_;

    static void callbackShowJoinLeaveInChannels(Fl_Widget*, void*);
    static void callbackApply(Fl_Widget*, void*);

    void loadPrefs();
    void savePrefs();
};
