// This file is part of flobby (GPL v2 or later), see the LICENSE file

#include "ChannelChatTab.h"
#include "UserList.h"
#include "TextDisplay2.h"
#include "ChatInput.h"
#include "ITabs.h"
#include "Prefs.h"
#include "ChatSettingsDialog.h"
#include "Sound.h"
#include "TextFunctions.h"

#include "model/Model.h"
#include "log/Log.h"

#include <FL/Fl.H>
#include <boost/algorithm/string.hpp>
#include <boost/bind.hpp>

// we use the split setting of the ServerMessages tab in all channel tabs
static char const * PrefServerMessagesSplitH = "ServerMessagesSplitH";

ChannelChatTab::ChannelChatTab(int x, int y, int w, int h, std::string const & channelName,
                         ITabs& iTabs, Model & model, ChatSettingsDialog & chatSettingsDialog):
    Fl_Tile(x,y,w,h),
    iTabs_(iTabs),
    model_(model),
    chatSettingsDialog_(chatSettingsDialog),
    channelName_(channelName),
    logFile_("channel_" + channelName)
{
    // make tab name unique
    std::string const tabName("#"+channelName);
    copy_label(tabName.c_str());

    // left side (text and input)
    int const leftW = 0.75*w;
    Fl_Group * left = new Fl_Group(x, y, leftW, h);
    int const ih = FL_NORMAL_SIZE*2; // input height
    text_ = new TextDisplay2(x, y, leftW, h-ih);
    input_ = new ChatInput(x, y+h-ih, leftW, ih);
    input_->connectText( boost::bind(&ChannelChatTab::onInput, this, _1) );
    input_->connectComplete( boost::bind(&ChannelChatTab::onComplete, this, _1, _2, _3) );
    left->resizable(text_);
    left->end();

    // right side (user list)
    int const rightW = w - leftW;
    userList_ = new UserList(x+leftW, y, rightW, h, model_, iTabs_);

    // setup split
    {
        int x;
        prefs().get(PrefServerMessagesSplitH, x, 0);
        if (x != 0)
        {
            position(userList_->x(), 0, x, 0);
        }
    }

    end();

    chatSettingsDialog_.connectChatSettingsChanged( boost::bind(&ChannelChatTab::initChatSettings, this) );
    initChatSettings();

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
        labelcolor(FL_FOREGROUND_COLOR);
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
        append(message, 0);
    }
}

void ChannelChatTab::clients(std::string const & channelName, std::vector<std::string> const & clients)
{
    if (channelName == channelName_)
    {
        for (std::string const & userName : clients)
        {
            // catch non-existing user exception here (uberserver bug) to not skip the rest of users in channel
            try
            {
                userList_->add(userName);
            }
            catch (std::invalid_argument const& ex)
            {
                LOG(WARNING)<< ex.what();
            }
        }
    }
}

void ChannelChatTab::userJoined(std::string const & channelName, std::string const & userName)
{
    if (channelName == channelName_)
    {
        userList_->add(userName);

        if (showJoinLeave_)
        {
            std::ostringstream oss;
            oss << userName << " joined";

            append(oss.str());
        }
    }
}

void ChannelChatTab::userLeft(std::string const & channelName, std::string const & userName, std::string const & reason)
{
    if (channelName == channelName_)
    {
        userList_->remove(userName);

        if (showJoinLeave_)
        {
            std::ostringstream oss;
            oss << userName << " left";

            if (!reason.empty())
            {
                oss << " (" << reason << ")";
            }
            append(oss.str());
        }
    }
}

void ChannelChatTab::said(std::string const & channelName, std::string const & userName, std::string const & message)
{
    if (channelName == channelName_)
    {
        int interest = 0;
        std::string const& myName = model_.me().name();
        if (userName == myName)
        {
            interest = -2;
        }
        else if (message.find(myName) != std::string::npos)
        {
            interest = 1;
        }

        append(userName + ": " + message, interest);
    }
}

void ChannelChatTab::leave()
{
    model_.leaveChannel(channelName_);
    text_->append("", -1); // add empty line when leaving
    userList_->clear();
}

void ChannelChatTab::append(std::string const & msg, int interest)
{
    if (msg.empty())
    {
        text_->append(msg, -1);
        return;
    }

    logFile_.log(msg);

    text_->append(msg, interest);

    // make ChatTabs redraw header
    if (interest >= 0 && !visible() && labelcolor() != FL_RED)
    {
        labelcolor(FL_RED);
        iTabs_.redrawTabs();
    }

    if ((interest == 0 && beep_) || interest > 0)
    {
        Sound::beep();
    }

}

void ChannelChatTab::initChatSettings()
{
    ChannelChatSettings const & settings = chatSettingsDialog_.getChannelChatSettings();

    showJoinLeave_ = settings.showJoinLeave;

    // compare case insensitive
    bool const isException = std::find_if(
            settings.beepExceptions.begin(), settings.beepExceptions.end(),
            [this](std::string const & val) { return boost::iequals(val, this->channelName_); } ) // using lambda
//            boost::bind(&boost::iequals<std::string,std::string>, _1, channelName_, std::locale())) // using bind
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

void ChannelChatTab::onComplete(std::string const& text, std::size_t pos, std::pair<std::string, std::size_t>& result)
{
    auto const pairWordPos = getLastWord(text, pos);

    std::string const userName = userList_->completeUserName(pairWordPos.first);

    if (!userName.empty())
    {
        result.first = text.substr(0, pairWordPos.second) + userName + text.substr(pos);
        result.second = pairWordPos.second + userName.length();
    }
}
