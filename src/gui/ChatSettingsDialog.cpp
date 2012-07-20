#include "ChatSettingsDialog.h"
#include "ChannelChatTab.h"
#include "Prefs.h"

#include "log/Log.h"

#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Return_Button.H>

// Prefs
static char const * const PrefShowJoinLeaveInChannels = "ShowJoinLeaveInChannels";

ChatSettingsDialog::ChatSettingsDialog():
    Fl_Window(400, 400, "Chat settings")
{
    set_modal();

    showJoinLeaveInChannels_ = new Fl_Check_Button(10, 30, 380, 30, "Show join and leave messages in channels");

    Fl_Return_Button * btn = new Fl_Return_Button(300, 360, 90, 30, "Apply");
    btn->callback(ChatSettingsDialog::callbackApply, this);

    end();

    loadPrefs();
    savePrefs();
}

ChatSettingsDialog::~ChatSettingsDialog()
{
}

void ChatSettingsDialog::show()
{
    loadPrefs();
    Fl_Window::show();
}

void ChatSettingsDialog::loadPrefs()
{
    int val;
    prefs.get(PrefShowJoinLeaveInChannels, val, 0);
    showJoinLeaveInChannels_->value(val);
}

void ChatSettingsDialog::savePrefs()
{
    int const val = showJoinLeaveInChannels_->value();
    prefs.set(PrefShowJoinLeaveInChannels, val);
    ChannelChatTab::showJoinLeave_ = (val == 1) ? true : false;
}

void ChatSettingsDialog::callbackApply(Fl_Widget*, void *data)
{
    ChatSettingsDialog * o = static_cast<ChatSettingsDialog*>(data);
    o->savePrefs();
    o->hide();
}

