#include "PrivateChat.h"
#include "TextDisplay.h"
#include "IChatTabs.h"

#include "model/Model.h"

#include <FL/Fl_Input.H>
#include <FL/Fl.H>
#include <FL/fl_ask.H>
#include <boost/bind.hpp>
#include <boost/algorithm/string.hpp>

PrivateChat::PrivateChat(int x, int y, int w, int h, std::string const & userName,
                         IChatTabs& iChatTabs, Model & model):
    Fl_Group(x,y,w,h),
    userName_(userName),
    iChatTabs_(iChatTabs),
    model_(model)
{
    // make tab name unique
    std::string const tabName(" "+userName_);
    copy_label(tabName.c_str());

    int const m = 0; // margin
    int const ih = 24; // input height
    text_ = new TextDisplay(x+m, y+m, w-2*m, h-ih-2*m);

    input_ = new Fl_Input(x, y+h-ih, w, ih);
    input_->callback(PrivateChat::onInput, this);
    input_->when(FL_WHEN_ENTER_KEY);

    resizable(text_);

    end();

    // model signals
    model_.connectSayPrivate( boost::bind(&PrivateChat::say, this, _1, _2) );
    model_.connectSaidPrivate( boost::bind(&PrivateChat::said, this, _1, _2) );
    model_.connectUserJoined( boost::bind(&PrivateChat::userJoined, this, _1) );
    model_.connectUserLeft( boost::bind(&PrivateChat::userLeft, this, _1) );

    Fl::focus(input_);
}

PrivateChat::~PrivateChat()
{
}

int PrivateChat::handle(int event)
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

void PrivateChat::onInput(Fl_Widget * w, void * data)
{
    PrivateChat * pc = static_cast<PrivateChat*>(data);

    std::string msg(pc->input_->value());
    boost::trim(msg);

    if (!msg.empty())
    {
        pc->model_.sayPrivate(pc->userName_, msg);
    }
    pc->input_->value("");
}

void PrivateChat::say(std::string const & userName, std::string const & msg)
{
    if (userName == userName_)
    {
        std::ostringstream oss;
        oss << "@C" << FL_DARK2 << "@." << msg;
        append(oss.str());
    }
}

void PrivateChat::said(std::string const & userName, std::string const & msg)
{
    if (userName == userName_)
    {
        append(userName + ": " + msg, true);
        fl_beep();
    }
}

void PrivateChat::userJoined(User const & user)
{
    if (user.name() == userName_)
    {
        std::ostringstream oss;
        oss << "@C" << FL_DARK2 << "@." << "user joined server";
        append(oss.str(), true);
    }
}

void PrivateChat::userLeft(User const & user)
{
    if (user.name() == userName_)
    {
        std::ostringstream oss;
        oss << "@C" << FL_DARK2 << "@." << "user left server";
        append(oss.str(), true);
    }
}

void PrivateChat::append(std::string const & msg, bool interesting)
{
    text_->append(msg);
    // make ChatTabs redraw header
    if (interesting && !visible() && labelcolor() != FL_RED)
    {
        labelcolor(FL_RED);
        iChatTabs_.redrawTabs();
    }

}
