#pragma once

#include <FL/Fl_Window.H>
#include <boost/signals2.hpp>
#include <vector>
#include <string>

class Fl_Check_Button;
class Fl_Multiline_Input;

struct ChannelChatSettings
{
    bool showJoinLeave;
    bool beep;
    std::vector<std::string> beepExceptions;
};

struct PrivateChatSettings
{
    bool beep;
    std::vector<std::string> beepExceptions;
};

class ChatSettingsDialog: public Fl_Window
{
public:
    ChatSettingsDialog();
    virtual ~ChatSettingsDialog();

    void show();

    ChannelChatSettings const & getChannelChatSettings() { return channelChatSettings_; }
    PrivateChatSettings const & getPrivateChatSettings() { return privateChatSettings_; }

    // signals
    typedef boost::signals2::signal<void (void)> ChatSettingsChangedSignal;
    boost::signals2::connection connectChatSettingsChanged(ChatSettingsChangedSignal::slot_type subscriber)
    { return ChatSettingsChangedSignal_.connect(subscriber); }

private:
    Fl_Check_Button * showJoinLeaveInChannels_;
    Fl_Check_Button * channelChatBeep_;
    Fl_Multiline_Input * channelChatBeepExceptions_;

    Fl_Check_Button * privateChatBeep_;
    Fl_Multiline_Input * privateChatBeepExceptions_;

    ChannelChatSettings channelChatSettings_;
    PrivateChatSettings privateChatSettings_;

    static void callbackApply(Fl_Widget*, void*);

    void loadPrefs();
    void savePrefs();
    void setCurrentSettings();

    ChatSettingsChangedSignal ChatSettingsChangedSignal_;
};
