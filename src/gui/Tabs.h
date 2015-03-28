// This file is part of flobby (GPL v2 or later), see the LICENSE file

#pragma once

#include "ITabs.h"

#include <FL/Fl_Tabs.H>
#include <map>
#include <string>

class ServerTab;
class ChannelChatTab;
class PrivateChatTab;
class ChatSettingsDialog;
class Model;
class Fl_Tabs;

class Tabs: public Fl_Tabs, public ITabs
{
public:
    Tabs(int x, int y, int w, int h, Model & model);
    virtual ~Tabs();

    void initTiles();
    void setChatSettingsDialog(ChatSettingsDialog * chatSettingsDialog) { chatSettingsDialog_ = chatSettingsDialog; } // ugly dependency injection

private:
    Model & model_;
    ChatSettingsDialog * chatSettingsDialog_;

    ServerTab * logUsersTab_;

    typedef std::map<std::string, PrivateChatTab*> PrivateChatTabs;
    PrivateChatTabs privateChatTabs_;

    typedef std::map<std::string, ChannelChatTab*> ChannelChatTabs;
    ChannelChatTabs channelChatTabs_;

    template <typename M>
    typename M::mapped_type createChat(std::string const & name, M & map); // used to create both private and channel tabs
    bool closeChat(Fl_Widget* w); // returns true if chat closed

    // model signal handlers
    void connected(bool connected);
    void saidPrivate(std::string const & userName, std::string const & msg); // msg from other, needed here to create new private chat tabs
    void channelJoined(std::string const & channelName);

    // ITabs
    void openPrivateChat(std::string const & userName);
    void openChannelChat(std::string const & channelName);
    void redrawTabs();

    int handle(int event);
    int handlePrivateChatClick(PrivateChatTab* pc);
    int handleChannelChatClick(ChannelChatTab* cc);
    void closeTab(Fl_Widget* w);
    void draw();
    void openLogFile(std::string const& path);
};
