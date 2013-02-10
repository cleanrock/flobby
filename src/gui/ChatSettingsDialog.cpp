#include "ChatSettingsDialog.h"
#include "TextDisplay2.h"
#include "Prefs.h"

#include "log/Log.h"

#include <FL/Fl_Box.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Multiline_Input.H>
#include <FL/Fl_Color_Chooser.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl.H>
#include <boost/algorithm/string.hpp>

// Prefs
static char const * const PrefShowJoinLeaveInChannels = "ShowJoinLeaveInChannels";
static char const * const PrefChannelChatBeep = "ChannelChatBeep";
static char const * const PrefChannelChatBeepExceptions = "ChannelChatBeepExceptions";
static char const * const PrefPrivateChatBeep = "PrivateChatBeep";
static char const * const PrefPrivateChatBeepExceptions = "PrivateChatBeepExceptions";

static char const * const PrefTimeColor = "TimeColor";
static char const * const PrefLowInterestColor = "LowInterestColor";
static char const * const PrefNormalInterestColor = "NormalInterestColor";
static char const * const PrefHighInterestColor = "HighInterestColor";
static char const * const PrefMyTextColor = "MyTextColor";

ChatSettingsDialog::ChatSettingsDialog():
    Fl_Window(800, 400, "Chat settings")
{
    set_modal();

    showJoinLeaveInChannels_ = new Fl_Check_Button(10, 30, 380, 30, "Show join and leave messages in channels");

    channelChatBeep_= new Fl_Check_Button(10, 60, 380, 30, "Channel chat beep");

    channelChatBeepExceptions_ = new Fl_Multiline_Input(10, 110, 380, 50, "Channel chat beep exceptions (channel names)");
    channelChatBeepExceptions_->align(FL_ALIGN_TOP_LEFT);

    privateChatBeep_= new Fl_Check_Button(10, 170, 380, 30, "Private chat beep");

    privateChatBeepExceptions_ = new Fl_Multiline_Input(10, 220, 380, 50, "Private chat beep exceptions (user names)");
    privateChatBeepExceptions_->align(FL_ALIGN_TOP_LEFT);

    {
        int index;
        int y = 30;

        index = TextDisplay2::STYLE_TIME;
        setTextColor_[index] = new Fl_Button(410, y, 200, 30, "Set Time color ...");
        textColor_[index] = new Fl_Box(FL_BORDER_BOX, 620, y, 30, 30, 0);
        setTextColor_[index]->callback(ChatSettingsDialog::callbackTimeColor, this);
        y += 40;

        index = TextDisplay2::STYLE_LOW;
        setTextColor_[index] = new Fl_Button(410, y, 200, 30, "Set Low Interest color ...");
        textColor_[index] = new Fl_Box(FL_BORDER_BOX, 620, y, 30, 30, 0);
        setTextColor_[index]->callback(ChatSettingsDialog::callbackLowColor, this);
        y += 40;

        index = TextDisplay2::STYLE_NORMAL;
        setTextColor_[index] = new Fl_Button(410, y, 200, 30, "Set Normal Interest color ...");
        textColor_[index] = new Fl_Box(FL_BORDER_BOX, 620, y, 30, 30, 0);
        setTextColor_[index]->callback(ChatSettingsDialog::callbackNormalColor, this);
        y += 40;

        index = TextDisplay2::STYLE_HIGH;
        setTextColor_[index] = new Fl_Button(410, y, 200, 30, "Set High Interest color ...");
        textColor_[index] = new Fl_Box(FL_BORDER_BOX, 620, y, 30, 30, 0);
        setTextColor_[index]->callback(ChatSettingsDialog::callbackHighColor, this);
        y += 40;

        index = TextDisplay2::STYLE_MYTEXT;
        setTextColor_[index] = new Fl_Button(410, y, 200, 30, "Set my text color ...");
        textColor_[index] = new Fl_Box(FL_BORDER_BOX, 620, y, 30, 30, 0);
        setTextColor_[index]->callback(ChatSettingsDialog::callbackMyTextColor, this);
        y += 40;

        y += 20;
        chatSample_ = new TextDisplay2(410, y, 380, 65, "Chat sample (FLTK will force black or white if contrast is low)");
        chatSample_->append("Low Interest", -1);
        chatSample_->append("Normal Interest", 0);
        chatSample_->append("High Interest", 1);
        chatSample_->append("My text", -2);
    }

    Fl_Return_Button * btn = new Fl_Return_Button(700, 360, 90, 30, "Apply");
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

    // chat text color
    {
        prefs.get(PrefTimeColor, val, FL_INACTIVE_COLOR);
        textColor_[TextDisplay2::STYLE_TIME]->color(val);

        prefs.get(PrefLowInterestColor, val, FL_INACTIVE_COLOR);
        textColor_[TextDisplay2::STYLE_LOW]->color(val);

        prefs.get(PrefNormalInterestColor, val, FL_FOREGROUND_COLOR);
        textColor_[TextDisplay2::STYLE_NORMAL]->color(val);

        prefs.get(PrefHighInterestColor, val, FL_FOREGROUND_COLOR);
        textColor_[TextDisplay2::STYLE_HIGH]->color(val);

        prefs.get(PrefMyTextColor, val, FL_FOREGROUND_COLOR);
        textColor_[TextDisplay2::STYLE_MYTEXT]->color(val);
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

    // chat text color
    {
        prefs.set(PrefTimeColor, static_cast<int>(textColor_[TextDisplay2::STYLE_TIME]->color()) );
        prefs.set(PrefLowInterestColor, static_cast<int>(textColor_[TextDisplay2::STYLE_LOW]->color()) );
        prefs.set(PrefNormalInterestColor, static_cast<int>(textColor_[TextDisplay2::STYLE_NORMAL]->color()) );
        prefs.set(PrefHighInterestColor, static_cast<int>(textColor_[TextDisplay2::STYLE_HIGH]->color()) );
        prefs.set(PrefMyTextColor, static_cast<int>(textColor_[TextDisplay2::STYLE_MYTEXT]->color()) );
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

    // chat text color
    for (int i=0; i<TextDisplay2::STYLE_COUNT; ++i)
    {
        TextDisplay2::textStyles_[i].color = textColor_[i]->color();
    }
}

void ChatSettingsDialog::selectColor(std::string const& title, int index)
{
    uchar r,g,b;
    Fl::get_color(TextDisplay2::textStyles_[index].color, r,g,b);
    if (fl_color_chooser(title.c_str(), r, g, b))
    {
        Fl_Color color = fl_rgb_color(r, g, b);
        TextDisplay2::textStyles_[index].color = color;
        textColor_[index]->color(color);
        Fl::redraw();
    }
}

void ChatSettingsDialog::callbackTimeColor(Fl_Widget* w, void *data)
{
    ChatSettingsDialog * o = static_cast<ChatSettingsDialog*>(data);
    o->selectColor("Time", TextDisplay2::STYLE_TIME);
}

void ChatSettingsDialog::callbackLowColor(Fl_Widget* w, void *data)
{
    ChatSettingsDialog * o = static_cast<ChatSettingsDialog*>(data);
    o->selectColor("Low", TextDisplay2::STYLE_LOW);
}

void ChatSettingsDialog::callbackNormalColor(Fl_Widget* w, void *data)
{
    ChatSettingsDialog * o = static_cast<ChatSettingsDialog*>(data);
    o->selectColor("Normal", TextDisplay2::STYLE_NORMAL);
}

void ChatSettingsDialog::callbackHighColor(Fl_Widget* w, void *data)
{
    ChatSettingsDialog * o = static_cast<ChatSettingsDialog*>(data);
    o->selectColor("High", TextDisplay2::STYLE_HIGH);
}

void ChatSettingsDialog::callbackMyTextColor(Fl_Widget* w, void *data)
{
    ChatSettingsDialog * o = static_cast<ChatSettingsDialog*>(data);
    o->selectColor("My text", TextDisplay2::STYLE_MYTEXT);
}

void ChatSettingsDialog::callbackApply(Fl_Widget*, void *data)
{
    ChatSettingsDialog * o = static_cast<ChatSettingsDialog*>(data);
    o->savePrefs();
    o->hide();
}

