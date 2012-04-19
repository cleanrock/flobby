#include "ChatTabs.h"
#include "Prefs.h"
#include "StringTable.h"
#include "TextDisplay.h"
#include "ServerMessages.h"
#include "ChannelChat.h"
#include "PrivateChat.h"
#include "PopupMenu.h"

#include "model/Model.h"

#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Tile.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Input.H>
#include <FL/fl_ask.H>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <sstream>


ChatTabs::ChatTabs(int x, int y, int w, int h, Model & model):
    Fl_Tabs(x,y,w,h),
    model_(model)
{
    selection_color(FL_LIGHT2);

    client_area(x,y,w,h);
    int const off = 0;
    y += off; h -= off;
    server_ = new ServerMessages(x,y,w,h, *this, *this, model_);
    add(server_);

    resizable(server_);

    end();

    // model signals
    model_.connectSaidPrivate( boost::bind(&ChatTabs::saidPrivate, this, _1, _2) );
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
    Type * chat = new Type(x,y,w,h, name, *this, model_);
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

        // re-add it if it was closed
        if (find(pc) == children())
        {
            add(pc);
        }

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
        ChannelChat * cc = it->second;

        // re-add it if it was closed
        if (find(cc) == children())
        {
            add(cc);
        }

        value(cc);
        cc->show();
    }

}

void ChatTabs::saidPrivate(std::string const & userName, std::string const & msg)
{
    auto it = privateChats_.find(userName);
    if (it == privateChats_.end())
    {
        createChat(userName, privateChats_);
    }
    else
    {
        PrivateChat * pc = it->second;
        // re-add it if it was closed
        if (find(pc) == children())
        {
            add(pc);
        }
    }
}

void ChatTabs::channelJoined(std::string const & channelName)
{
    openChannelChat(channelName);
}

int ChatTabs::handle(int event)
{
    // close chats and channels (left double clicking or context menu)
    // channels are left when closed
    // chats are not destroyed, they will be just be re-added when needed
    if (event == FL_PUSH)
    {
        Fl_Widget* chat =  which(Fl::event_x(), Fl::event_y());

        if (chat != 0 && chat != server_)
        {
            if (Fl::event_button() == FL_LEFT_MOUSE && Fl::event_clicks())
            {
                if (closeChat(chat))
                {
                    return 1;
                }
            }
            else if (Fl::event_button() == FL_RIGHT_MOUSE && Fl::event_clicks() == 0)
            {
                PopupMenu menu;
                menu.add("Close (Leave)", 1);
                int const id = menu.show();
                switch (id)
                {
                case 1:
                    if (closeChat(chat))
                    {
                        return 1;
                    }
                    break;
                }

            }
        }
    }

    return Fl_Tabs::handle(event);
}

void ChatTabs::draw()
{
    // workaround for bugged drawing in Fl_Tabs, needed when closing tabs
    fl_color(FL_BACKGROUND_COLOR);
    fl_rectf(x(), y(), w(), server_->y() - y());

    Fl_Tabs::draw();
}

bool ChatTabs::closeChat(Fl_Widget* w)
{
    for (auto & pair : privateChats_)
    {
        if (pair.second == w)
        {
            remove(w);
            redraw();
            return true;
        }
    }

    for (auto & pair : channelChats_)
    {
        if (pair.second == w)
        {
            remove(w);
            ChannelChat * cc = static_cast<ChannelChat*>(w);
            cc->leave();
            redraw();
            return true;
        }
    }

    return false;
}

void ChatTabs::redrawTabs()
{
    redraw_tabs();
}
