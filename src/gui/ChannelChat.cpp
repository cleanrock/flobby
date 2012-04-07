#include "ChannelChat.h"
#include "TextDisplay.h"
#include "Prefs.h"

#include "model/Model.h"

#include <FL/Fl_Input.H>
#include <FL/Fl.H>
#include <boost/algorithm/string.hpp>
#include <boost/bind.hpp>

// we use the split setting of the ServerMessages tab in all channel tabs
static char const * PrefServerMessagesSplitH = "ServerMessagesSplitH";

ChannelChat::ChannelChat(int x, int y, int w, int h, std::string const & channelName, Model & model):
    Fl_Tile(x,y,w,h),
    model_(model),
    channelName_(channelName)
{
    // make tab name unique
    std::string const tabName("#"+channelName);
    copy_label(tabName.c_str());

    // left side (text and input)
    int const leftW = 0.75*w;
    Fl_Group * left = new Fl_Group(x, y, leftW, h);
    int const ih = 24; // input height
    text_ = new TextDisplay(x, y, leftW, h-ih);
    input_ = new Fl_Input(x, y+h-ih, leftW, ih);
    input_->callback(ChannelChat::onInput, this);
    input_->when(FL_WHEN_ENTER_KEY);
    left->resizable(text_);
    left->end();

    // right side (user list)
    int const rightW = w - leftW;
    userList_ = new StringTable(x+leftW, y, rightW, h, "ChannelUserList",
            { "name" });

    // setup split
    {
        int x;
        prefs.get(PrefServerMessagesSplitH, x, 0);
        if (x != 0)
        {
            position(userList_->x(), 0, x, 0);
        }
    }

    end();

    // model signals
    model_.connectChannelTopicSignal( boost::bind(&ChannelChat::topic, this, _1, _2, _3, _4) );
    model_.connectChannelClients( boost::bind(&ChannelChat::clients, this, _1, _2) );
    model_.connectUserJoinedChannel( boost::bind(&ChannelChat::userJoined, this, _1, _2) );
    model_.connectUserLeftChannel( boost::bind(&ChannelChat::userLeft, this, _1, _2, _3) );
    model_.connectSaidChannel( boost::bind(&ChannelChat::said, this, _1, _2, _3) );

}

ChannelChat::~ChannelChat()
{
}

int ChannelChat::handle(int event)
{
    switch (event)
    {
    case FL_SHOW:
        Fl::focus(input_);
        break;
    }
    return Fl_Tile::handle(event);
}

void ChannelChat::onInput(Fl_Widget * w, void * data)
{
    ChannelChat * cc = static_cast<ChannelChat*>(data);

    std::string msg(cc->input_->value());
    boost::trim(msg);

    if (!msg.empty())
    {
        cc->model_.sayChannel(cc->channelName_, msg);

        // always scroll on input
        // TODO
    }
    cc->input_->value("");
}

StringTableRow ChannelChat::makeRow(std::string const & userName)
{
    return StringTableRow( userName,
        {
            userName
        } );
}

std::string ChannelChat::flagsString(User const & user)
{
    std::ostringstream oss;
    oss << (user.status().inGame() ? "G" : "");
    return oss.str();
}

void ChannelChat::topic(std::string const & channelName, std::string const & author, time_t epochSeconds, std::string const & topic)
{
    if (channelName == channelName_)
    {
        text_->append("Topic: " + topic);

        std::string timeString(ctime(&epochSeconds));
        boost::algorithm::erase_all(timeString, "\n");

        text_->append("Topic set " + timeString + " by " + author);
    }
}

void ChannelChat::clients(std::string const & channelName, std::vector<std::string> const & clients)
{
    if (channelName == channelName_)
    {
        for (std::string const & userName : clients)
        {
            userList_->addRow(makeRow(userName));
        }
    }
}

void ChannelChat::userJoined(std::string const & channelName, std::string const & userName)
{
    if (channelName == channelName_)
    {
        userList_->addRow(makeRow(userName));
        text_->append(userName + " joined");
    }
}

void ChannelChat::userLeft(std::string const & channelName, std::string const & userName, std::string const & reason)
{
    if (channelName == channelName_)
    {
        userList_->removeRow(userName);
        if (reason.empty())
        {
            text_->append(userName + " left ");
        }
        else
        {
            text_->append(userName + " left (" + reason + ")");
        }
    }
}

void ChannelChat::said(std::string const & channelName, std::string const & userName, std::string const & message)
{
    if (channelName == channelName_)
    {
        text_->append(userName + ": " + message);
    }
}

