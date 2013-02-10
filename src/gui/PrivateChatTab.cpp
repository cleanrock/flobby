#include "PrivateChatTab.h"
#include "TextDisplay2.h"
#include "ChatInput.h"
#include "ITabs.h"
#include "Sound.h"
#include "ChatSettingsDialog.h"

#include "model/Model.h"

#include <FL/Fl.H>
#include <FL/fl_ask.H>
#include <boost/bind.hpp>
#include <boost/algorithm/string.hpp>

PrivateChatTab::PrivateChatTab(int x, int y, int w, int h, std::string const & userName,
                         ITabs& iTabs, Model & model, ChatSettingsDialog & chatSettingsDialog):
    Fl_Group(x,y,w,h),
    userName_(userName),
    iTabs_(iTabs),
    model_(model),
    chatSettingsDialog_(chatSettingsDialog),
    logFile_("chat_" + userName)
{
    // make tab name unique
    std::string const tabName(" "+userName_);
    copy_label(tabName.c_str());

    int const m = 0; // margin
    int const ih = 24; // input height
    text_ = new TextDisplay2(x+m, y+m, w-2*m, h-ih-2*m);

    input_ = new ChatInput(x, y+h-ih, w, ih);
    input_->connectText( boost::bind(&PrivateChatTab::onInput, this, _1) );

    resizable(text_);

    end();

    chatSettingsDialog_.connectChatSettingsChanged( boost::bind(&PrivateChatTab::initChatSettings, this) );
    initChatSettings();

    // model signals
    model_.connectSayPrivate( boost::bind(&PrivateChatTab::say, this, _1, _2) );
    model_.connectSaidPrivate( boost::bind(&PrivateChatTab::said, this, _1, _2) );
    model_.connectUserJoined( boost::bind(&PrivateChatTab::userJoined, this, _1) );
    model_.connectUserLeft( boost::bind(&PrivateChatTab::userLeft, this, _1) );

    model_.connectUserJoinedBattle( boost::bind(&PrivateChatTab::userJoinedBattle, this, _1, _2) );
    model_.connectUserLeftBattle( boost::bind(&PrivateChatTab::userLeftBattle, this, _1, _2) );

    Fl::focus(input_);
}

PrivateChatTab::~PrivateChatTab()
{
}

int PrivateChatTab::handle(int event)
{
    switch (event)
    {
    case FL_SHOW:
        labelcolor(FL_FOREGROUND_COLOR);
        Fl::focus(input_);
        break;
    }
    return Fl_Group::handle(event);
}

void PrivateChatTab::onInput(std::string const & text)
{
    model_.sayPrivate(userName_, text);
}

void PrivateChatTab::say(std::string const & userName, std::string const & msg)
{
    if (userName == userName_)
    {
        append(msg, -2); // my text
    }
}

void PrivateChatTab::said(std::string const & userName, std::string const & msg)
{
    if (userName == userName_)
    {
        append(userName + ": " + msg, 0); // normal
    }
}

void PrivateChatTab::userJoined(User const & user)
{
    if (user.name() == userName_)
    {
        append(userName_ + " joined server", -1);
    }
}

void PrivateChatTab::initChatSettings()
{
    PrivateChatSettings const & settings = chatSettingsDialog_.getPrivateChatSettings();

    // compare case insensitive
    bool const isException = std::find_if(
            settings.beepExceptions.begin(), settings.beepExceptions.end(),
            [this](std::string const & val) { return boost::iequals(val, this->userName_); } ) // using lambda
//            boost::bind(&boost::iequals<std::string,std::string>, _1, userName_, std::locale())) // using bind
                != settings.beepExceptions.end();

    if ( (settings.beep && !isException) || (!settings.beep && isException) )
    {
        beep_ = true;
    }
    else
    {
        beep_ = false;
    }
}

void PrivateChatTab::userLeft(User const & user)
{
    if (user.name() == userName_)
    {
        append(userName_ + " left server", -1);
    }
}

void PrivateChatTab::userJoinedBattle(User const & user, Battle const & battle)
{
    if (user.name() == userName_)
    {
        append(userName_ + " joined " + battle.title(), -1);
    }
}

void PrivateChatTab::userLeftBattle(User const & user, Battle const & battle)
{
    if (user.name() == userName_)
    {
        append(userName_ + " left " + battle.title(), -1);
    }
}

void PrivateChatTab::append(std::string const & msg, int interest)
{
    logFile_.log(msg);

    // interest can be :
    // -2 my text
    // -1 join/leave messages
    // 0 other part sent text

    text_->append(msg, interest);

    // make Tabs redraw header if other part send text
    if (interest == 0 && !visible() && labelcolor() != FL_RED)
    {
        labelcolor(FL_RED);
        iTabs_.redrawTabs();
    }

    if (interest == 0 && beep_)
    {
        Sound::beep();
    }
}
