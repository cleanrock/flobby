#include "ChatSettingsDialog.h"
#include "Prefs.h"

#include "log/Log.h"

#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Multiline_Input.H>
#include <FL/Fl_Return_Button.H>
#include <boost/algorithm/string.hpp>

// Prefs
static char const * const PrefShowJoinLeaveInChannels = "ShowJoinLeaveInChannels";
static char const * const PrefChannelChatBeep = "ChannelChatBeep";
static char const * const PrefChannelChatBeepExceptions = "ChannelChatBeepExceptions";
static char const * const PrefPrivateChatBeep = "PrivateChatBeep";
static char const * const PrefPrivateChatBeepExceptions = "PrivateChatBeepExceptions";

ChatSettingsDialog::ChatSettingsDialog():
    Fl_Window(400, 400, "Chat settings")
{
    set_modal();

    showJoinLeaveInChannels_ = new Fl_Check_Button(10, 30, 380, 30, "Show join and leave messages in channels");

    channelChatBeep_= new Fl_Check_Button(10, 60, 380, 30, "Channel chat beep");

    channelChatBeepExceptions_ = new Fl_Multiline_Input(10, 110, 380, 50, "Channel chat beep exceptions (channel names)");
    channelChatBeepExceptions_->align(FL_ALIGN_TOP_LEFT);

    privateChatBeep_= new Fl_Check_Button(10, 170, 380, 30, "Private chat beep");

    privateChatBeepExceptions_ = new Fl_Multiline_Input(10, 220, 380, 50, "Private chat beep exceptions (user names)");
    privateChatBeepExceptions_->align(FL_ALIGN_TOP_LEFT);

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

    // channel chat
    {
        prefs.get(PrefShowJoinLeaveInChannels, val, 0);
        showJoinLeaveInChannels_->value(val);

        prefs.get(PrefChannelChatBeep, val, 0);
        channelChatBeep_->value(val);

        char * text;
        prefs.get(PrefChannelChatBeepExceptions, text, "");
        channelChatBeepExceptions_->value(text);
        ::free(text);
    }

    // private chat
    {
        prefs.get(PrefPrivateChatBeep, val, 1);
        privateChatBeep_->value(val);

        char * text;
        prefs.get(PrefPrivateChatBeepExceptions, text, "");
        privateChatBeepExceptions_->value(text);
        ::free(text);
    }

    setCurrentSettings();
}

void ChatSettingsDialog::savePrefs()
{
    namespace ba = boost::algorithm;

    // channel chat
    {
        int const showJoinLeave = showJoinLeaveInChannels_->value();
        prefs.set(PrefShowJoinLeaveInChannels, showJoinLeave);

        int const beep = channelChatBeep_->value();
        prefs.set(PrefChannelChatBeep, beep);

        std::string const beepExceptions(channelChatBeepExceptions_->value());
        prefs.set(PrefChannelChatBeepExceptions, beepExceptions.c_str());
    }

    // private chat
    {
        int const beep = privateChatBeep_->value();
        prefs.set(PrefPrivateChatBeep, beep);

        std::string const beepExceptions(privateChatBeepExceptions_->value());
        prefs.set(PrefPrivateChatBeepExceptions, beepExceptions.c_str());

    }

    setCurrentSettings();

    ChatSettingsChangedSignal_();
}

void ChatSettingsDialog::setCurrentSettings()
{
    namespace ba = boost::algorithm;

    // channel chat
    {
        channelChatSettings_.showJoinLeave = (showJoinLeaveInChannels_->value() == 1) ? true : false;

        channelChatSettings_.beep = (channelChatBeep_->value() == 1) ? true : false;

        std::vector<std::string> words;
        std::string const text = channelChatBeepExceptions_->value();
        ba::split( words, text, ba::is_any_of("\n "), ba::token_compress_on );
        words.erase( std::remove_if(words.begin(), words.end(), std::mem_fun_ref(&std::string::empty)), words.end() );
        channelChatSettings_.beepExceptions = words;

    }

    // private chat
    {
        privateChatSettings_.beep = (privateChatBeep_->value() == 1) ? true : false;

        std::vector<std::string> words;
        std::string const text = privateChatBeepExceptions_->value();
        ba::split( words, text, ba::is_any_of("\n "), ba::token_compress_on );
        words.erase( std::remove_if(words.begin(), words.end(), std::mem_fun_ref(&std::string::empty)), words.end() );
        privateChatSettings_.beepExceptions = words;
    }

}

void ChatSettingsDialog::callbackApply(Fl_Widget*, void *data)
{
    ChatSettingsDialog * o = static_cast<ChatSettingsDialog*>(data);
    o->savePrefs();
    o->hide();
}

