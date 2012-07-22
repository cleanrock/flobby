#include "ChannelChatTab.h"
#include "UserList.h"
#include "TextDisplay.h"
#include "ChatInput.h"
#include "ITabs.h"
#include "Prefs.h"

#include "model/Model.h"

#include <FL/Fl.H>
#include <boost/algorithm/string.hpp>
#include <boost/bind.hpp>

// we use the split setting of the ServerMessages tab in all channel tabs
static char const * PrefServerMessagesSplitH = "ServerMessagesSplitH";

bool ChannelChatTab::showJoinLeave_ = false;

ChannelChatTab::ChannelChatTab(int x, int y, int w, int h, std::string const & channelName,
                         ITabs& iTabs, Model & model):
    Fl_Tile(x,y,w,h),
    iTabs_(iTabs),
    model_(model),
    channelName_(channelName),
    logFile_("channel_" + channelName)
{
    // make tab name unique
    std::string const tabName("#"+channelName);
    copy_label(tabName.c_str());

    // left side (text and input)
    int const leftW = 0.75*w;
    Fl_Group * left = new Fl_Group(x, y, leftW, h);
    int const ih = 24; // input height
    text_ = new TextDisplay(x, y, leftW, h-ih);
    input_ = new ChatInput(x, y+h-ih, leftW, ih);
    input_->connectText( boost::bind(&ChannelChatTab::onInput, this, _1) );
    left->resizable(text_);
    left->end();

    // right side (user list)
    int const rightW = w - leftW;
    userList_ = new UserList(x+leftW, y, rightW, h, model_, iTabs_);

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
    model_.connectChannelTopicSignal( boost::bind(&ChannelChatTab::topic, this, _1, _2, _3, _4) );
    model_.connectChannelMessageSignal( boost::bind(&ChannelChatTab::message, this, _1, _2) );
    model_.connectChannelClients( boost::bind(&ChannelChatTab::clients, this, _1, _2) );
    model_.connectUserJoinedChannel( boost::bind(&ChannelChatTab::userJoined, this, _1, _2) );
    model_.connectUserLeftChannel( boost::bind(&ChannelChatTab::userLeft, this, _1, _2, _3) );
    model_.connectSaidChannel( boost::bind(&ChannelChatTab::said, this, _1, _2, _3) );

}

ChannelChatTab::~ChannelChatTab()
{
}

int ChannelChatTab::handle(int event)
{
    switch (event)
    {
    case FL_SHOW:
        labelcolor(FL_BLACK);
        Fl::focus(input_);
        break;
    }
    return Fl_Tile::handle(event);
}

void ChannelChatTab::onInput(std::string const & text)
{
    model_.sayChannel(channelName_, text);
}

void ChannelChatTab::topic(std::string const & channelName, std::string const & author, time_t epochSeconds, std::string const & topic)
{
    if (channelName == channelName_)
    {
        append("Topic: " + topic);

        std::string timeString(ctime(&epochSeconds));
        boost::algorithm::erase_all(timeString, "\n");

        append("Topic set " + timeString + " by " + author);
    }
}

void ChannelChatTab::message(std::string const & channelName, std::string const & message)
{
    if (channelName == channelName_)
    {
        append(message, true);
    }
}

void ChannelChatTab::clients(std::string const & channelName, std::vector<std::string> const & clients)
{
    if (channelName == channelName_)
    {
        for (std::string const & userName : clients)
        {
            userList_->add(userName);
        }
    }
}

void ChannelChatTab::userJoined(std::string const & channelName, std::string const & userName)
{
    if (showJoinLeave_ && channelName == channelName_)
    {
        userList_->add(userName);
        std::ostringstream oss;
        oss << userName << " joined";

        append(oss.str());
    }
}

void ChannelChatTab::userLeft(std::string const & channelName, std::string const & userName, std::string const & reason)
{
    if (showJoinLeave_ && channelName == channelName_)
    {
        userList_->remove(userName);

        std::ostringstream oss;
        oss << userName << " left";

        if (!reason.empty())
        {
            oss << " (" << reason << ")";
        }
        append(oss.str());
    }
}

void ChannelChatTab::said(std::string const & channelName, std::string const & userName, std::string const & message)
{
    if (channelName == channelName_)
    {
        append(userName + ": " + message, true);
    }
}

void ChannelChatTab::leave()
{
    model_.leaveChannel(channelName_);
    text_->clear();
    userList_->clear();
}

void ChannelChatTab::append(std::string const & msg, bool interesting)
{
    if (msg.empty())
    {
        text_->append(msg);
        return;
    }

    logFile_.log(msg);

    std::ostringstream oss;
    if (!interesting)
    {
        oss << "@C" << FL_DARK2 << "@.";
    }
    oss << msg;
    text_->append(oss.str());

    // make ChatTabs redraw header
    if (interesting && !visible() && labelcolor() != FL_RED)
    {
        labelcolor(FL_RED);
        iTabs_.redrawTabs();
    }

}
