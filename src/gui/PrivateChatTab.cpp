#include "PrivateChatTab.h"
#include "TextDisplay2.h"
#include "ChatInput.h"
#include "ITabs.h"
#include "Sound.h"

#include "model/Model.h"

#include <FL/Fl.H>
#include <FL/fl_ask.H>
#include <boost/bind.hpp>
#include <boost/algorithm/string.hpp>

PrivateChatTab::PrivateChatTab(int x, int y, int w, int h, std::string const & userName,
                         ITabs& iTabs, Model & model):
    Fl_Group(x,y,w,h),
    userName_(userName),
    iTabs_(iTabs),
    model_(model),
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

    // model signals
    model_.connectSayPrivate( boost::bind(&PrivateChatTab::say, this, _1, _2) );
    model_.connectSaidPrivate( boost::bind(&PrivateChatTab::said, this, _1, _2) );
    model_.connectUserJoined( boost::bind(&PrivateChatTab::userJoined, this, _1) );
    model_.connectUserLeft( boost::bind(&PrivateChatTab::userLeft, this, _1) );

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
        labelcolor(FL_BLACK);
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
        append(msg);
    }
}

void PrivateChatTab::said(std::string const & userName, std::string const & msg)
{
    if (userName == userName_)
    {
        append(userName + ": " + msg, true);
        Sound::beep();
    }
}

void PrivateChatTab::userJoined(User const & user)
{
    if (user.name() == userName_)
    {
        append(userName_ + " joined server");
    }
}

void PrivateChatTab::userLeft(User const & user)
{
    if (user.name() == userName_)
    {
        append(userName_ + " left server");
    }
}

void PrivateChatTab::append(std::string const & msg, bool interesting)
{
    logFile_.log(msg);

    text_->append(msg, interesting);

    // make Tabs redraw header
    if (interesting && !visible() && labelcolor() != FL_RED)
    {
        labelcolor(FL_RED);
        iTabs_.redrawTabs();
    }

}
