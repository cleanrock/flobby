#pragma once

#include "IChat.h"
#include "ITabs.h"

#include <FL/Fl_Tabs.H>
#include <map>
#include <string>

class LogUsersTab;
class ChannelChatTab;
class PrivateChatTab;
class Model;
class Fl_Tabs;

class Tabs: public Fl_Tabs, public IChat, public ITabs
{
public:
    Tabs(int x, int y, int w, int h, Model & model);
    virtual ~Tabs();

    void initTiles();

private:
    Model & model_;

    LogUsersTab * logUsersTab_;

    typedef std::map<std::string, PrivateChatTab*> PrivateChatTabs;
    PrivateChatTabs privateChatTabs_;

    typedef std::map<std::string, ChannelChatTab*> ChannelChatTabs;
    ChannelChatTabs channelChatTabs_;

    template <typename M>
    void createChat(std::string const & name, M & map); // used to create both private and channel tabs
    bool closeChat(Fl_Widget* w); // returns true if chat closed

    // model signal handlers
    void connected(bool connected);
    void saidPrivate(std::string const & userName, std::string const & msg); // msg from other, needed here to create new private chat tabs
    void channelJoined(std::string const & channelName);

    // IChat
    void openPrivateChat(std::string const & userName);
    void openChannelChat(std::string const & channelName);

    // ITabs (used by child windows)
    void redrawTabs();

    int handle(int event);
    void draw();
};
