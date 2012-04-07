#include "PrivateChat.h"
#include "TextDisplay.h"

#include "model/Model.h"

#include <FL/Fl_Input.H>
#include <FL/Fl.H>
#include <boost/bind.hpp>
#include <boost/algorithm/string.hpp>

PrivateChat::PrivateChat(int x, int y, int w, int h, std::string const & userName, Model & model):
    Fl_Group(x,y,w,h),
    userName_(userName),
    model_(model)
{
    // make tab name unique
    std::string const tabName(" "+userName_);
    copy_label(tabName.c_str());

    int const m = 1; // margin
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

        // always scroll on input
        // TODO pc->text_->show_insert_position();
    }
    pc->input_->value("");
}

void PrivateChat::say(std::string const & userName, std::string const & msg)
{
    if (userName == userName_)
    {
        text_->append(msg);
    }
}

void PrivateChat::said(std::string const & userName, std::string const & msg)
{
    if (userName == userName_)
    {
        text_->append(userName + ": " + msg);
    }
}
