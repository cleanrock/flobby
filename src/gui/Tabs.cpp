#include "Tabs.h"
#include "Prefs.h"
#include "StringTable.h"
#include "TextDisplay.h"
#include "LogUsersTab.h"
#include "ChannelChatTab.h"
#include "PrivateChatTab.h"
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


Tabs::Tabs(int x, int y, int w, int h, Model & model):
    Fl_Tabs(x,y,w,h),
    model_(model)
{
    selection_color(FL_LIGHT2);

    client_area(x,y,w,h);
    int const off = 0;
    y += off; h -= off;
    logUsersTab_ = new LogUsersTab(x,y,w,h, *this, *this, model_);
    add(logUsersTab_);

    resizable(logUsersTab_);

    end();

    // model signals
    model_.connectSaidPrivate( boost::bind(&Tabs::saidPrivate, this, _1, _2) );
    model_.connectChannelJoined( boost::bind(&Tabs::channelJoined, this, _1) );
}

Tabs::~Tabs()
{
}

void Tabs::initTiles()
{
    logUsersTab_->initTiles();
}

template <typename M>
void Tabs::createChat(std::string const & name, M & map)
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

void Tabs::openPrivateChat(std::string const & userName)
{
    PrivateChatTabs::const_iterator it = privateChatTabs_.find(userName);
    if (it == privateChatTabs_.end())
    {
        createChat(userName, privateChatTabs_);
    }
    else
    {
        assert(it->second);
        PrivateChatTab * pc = it->second;

        // re-add it if it was closed
        if (find(pc) == children())
        {
            add(pc);
        }

        value(pc);
        pc->show();
    }

}

void Tabs::openChannelChat(std::string const & channelName)
{
    auto it = channelChatTabs_.find(channelName);
    if (it == channelChatTabs_.end())
    {
        createChat(channelName, channelChatTabs_);
    }
    else
    {
        assert(it->second);
        ChannelChatTab * cc = it->second;

        // re-add it if it was closed
        if (find(cc) == children())
        {
            add(cc);
        }

        value(cc);
        cc->show();
    }

}

void Tabs::saidPrivate(std::string const & userName, std::string const & msg)
{
    auto it = privateChatTabs_.find(userName);
    if (it == privateChatTabs_.end())
    {
        createChat(userName, privateChatTabs_);
    }
    else
    {
        PrivateChatTab * pc = it->second;
        // re-add it if it was closed
        if (find(pc) == children())
        {
            add(pc);
        }
    }
}

void Tabs::channelJoined(std::string const & channelName)
{
    openChannelChat(channelName);
}

int Tabs::handle(int event)
{
    // close chats and channels (left double clicking or context menu)
    // channels are left when closed
    // chats are not destroyed, they will be just be re-added when needed
    if (event == FL_PUSH)
    {
        Fl_Widget* chat =  which(Fl::event_x(), Fl::event_y());

        if (chat != 0 && chat != logUsersTab_)
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

void Tabs::draw()
{
    // workaround for bugged drawing in Fl_Tabs, needed when closing tabs
    fl_color(FL_BACKGROUND_COLOR);
    fl_rectf(x(), y(), w(), logUsersTab_->y() - y());

    Fl_Tabs::draw();
}

bool Tabs::closeChat(Fl_Widget* w)
{
    for (auto & pair : privateChatTabs_)
    {
        if (pair.second == w)
        {
            remove(w);
            redraw();
            return true;
        }
    }

    for (auto & pair : channelChatTabs_)
    {
        if (pair.second == w)
        {
            remove(w);
            ChannelChatTab * cc = static_cast<ChannelChatTab*>(w);
            cc->leave();
            redraw();
            return true;
        }
    }

    return false;
}

void Tabs::redrawTabs()
{
    redraw_tabs();
}
