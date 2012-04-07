#include "ChatTabs.h"
#include "Prefs.h"
#include "StringTable.h"
#include "TextDisplay.h"
#include "ServerMessages.h"
#include "ChannelChat.h"
#include "PrivateChat.h"

#include "model/Model.h"

#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Tile.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Input.H>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <sstream>
#include <iostream>


ChatTabs::ChatTabs(int x, int y, int w, int h, Model & model):
    Fl_Tabs(x,y,w,h),
    model_(model)
{
    end();

    client_area(x,y,w,h);
    int const off = 0;
    y += off; h -= off;
    server_ = new ServerMessages(x,y,w,h, model_, *this);
    add(server_);

    resizable(server_);

    // model signals
    model_.connectSaidPrivate( boost::bind(&ChatTabs::said, this, _1, _2) );
    model_.connectChannelJoined( boost::bind(&ChatTabs::channelJoined, this, _1) );
}

ChatTabs::~ChatTabs()
{
}

void ChatTabs::initTiles()
{
    server_->initTiles();
}

template <typename M>
void ChatTabs::createChat(std::string const & name, M & map)
{
    // assumes chat do not exists, TODO remove check ???
    typename M::const_iterator it = map.find(name);
    assert(it == map.end());

    int x, y, w, h;
    client_area(x,y,w,h);
    typedef typename std::remove_pointer<typename M::mapped_type>::type Type;
    Type * chat = new Type(x,y,w,h, name, model_);
    add(chat);
    // pc->said(userName, msg); TODO remove, the signal will be sent to the newly created Chat also
    map[name] = chat;
    value(chat);
    chat->show(); // to get keyboard focus in input widget

}

void ChatTabs::openPrivateChat(std::string const & userName)
{
    PrivateChats::const_iterator it = privateChats_.find(userName);
    if (it == privateChats_.end())
    {
        createChat(userName, privateChats_);
    }
    else
    {
        assert(it->second);
        PrivateChat * pc = it->second;
        value(pc);
        pc->show();
    }

}

void ChatTabs::openChannelChat(std::string const & channelName)
{
    auto it = channelChats_.find(channelName);
    if (it == channelChats_.end())
    {
        createChat(channelName, channelChats_);
    }
    else
    {
        assert(it->second);
        ChannelChat * pc = it->second;
        value(pc);
        pc->show();
    }

}

void ChatTabs::said(std::string const & userName, std::string const & msg)
{
    auto it = privateChats_.find(userName);
    if (it == privateChats_.end())
    {
        createChat(userName, privateChats_);
    }
}

void ChatTabs::channelJoined(std::string const & channelName)
{
    openChannelChat(channelName);
}
