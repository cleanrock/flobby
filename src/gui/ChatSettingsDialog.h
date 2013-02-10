#pragma once

#include "TextDisplay2.h"

#include <FL/Fl_Window.H>
#include <boost/signals2.hpp>
#include <vector>
#include <string>

class Fl_Button;
class Fl_Check_Button;
class Fl_Multiline_Input;
class Fl_Box;

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

    Fl_Button* setTextColor_[TextDisplay2::STYLE_COUNT];
    Fl_Box* textColor_[TextDisplay2::STYLE_COUNT];
    TextDisplay2* chatSample_;
    void selectColor(std::string const& title, int index);
    static void callbackTimeColor(Fl_Widget* w, void* d);
    static void callbackLowColor(Fl_Widget* w, void* d);
    static void callbackNormalColor(Fl_Widget* w, void* d);
    static void callbackHighColor(Fl_Widget* w, void* d);
    static void callbackMyTextColor(Fl_Widget* w, void* d);

    static void callbackApply(Fl_Widget*, void*);

    void loadPrefs();
    void savePrefs();
    void setCurrentSettings();

    ChatSettingsChangedSignal ChatSettingsChangedSignal_;
};
