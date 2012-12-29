#include "LogUsersTab.h"
#include "TextDisplay2.h"
#include "UserList.h"
#include "Prefs.h"
#include "ITabs.h"
#include "PopupMenu.h"
#include "Sound.h"

#include "model/Model.h"

#include <FL/fl_ask.H>
#include <boost/bind.hpp>

static char const * PrefServerMessagesSplitH = "ServerMessagesSplitH";

LogUsersTab::LogUsersTab(int x, int y, int w, int h,
                               ITabs& iTabs, Model & model):
    Fl_Tile(x,y,w,h, "Log && Users"),
    iTabs_(iTabs),
    model_(model),
    logFile_("messages")
{
    text_ = new TextDisplay2(x, y, w/2, h);
    userList_ = new UserList(x+w/2, y, w/2, h, model_, iTabs_, true); // we save the prefs for UserList
    end();

    // model signals
    model_.connectConnected( boost::bind(&LogUsersTab::connected, this, _1) );
    model_.connectServerInfo( boost::bind(&LogUsersTab::serverInfo, this, _1) );
    model_.connectLoginResult( boost::bind(&LogUsersTab::loginResult, this, _1, _2) );
    model_.connectServerMsg( boost::bind(&LogUsersTab::message, this, _1) );
    model_.connectUserJoined( boost::bind(&LogUsersTab::userJoined, this, _1) );
    model_.connectUserLeft( boost::bind(&LogUsersTab::userLeft, this, _1) );
    model_.connectRing( boost::bind(&LogUsersTab::ring, this, _1) );
    model_.connectDownloadDone( boost::bind(&LogUsersTab::downloadDone, this, _1, _2) );
}

LogUsersTab::~LogUsersTab()
{
    prefs.set(PrefServerMessagesSplitH, userList_->x());
}

void LogUsersTab::initTiles()
{
    int x;
    prefs.get(PrefServerMessagesSplitH, x, 0);
    if (x != 0)
    {
        position(userList_->x(), 0, x, 0);
    }
}

void LogUsersTab::serverInfo(ServerInfo const & si)
{
    std::ostringstream oss;
    oss << "ServerInfo: " << si;
    append(oss.str());
}

void LogUsersTab::loginResult(bool success, std::string const & info)
{
    if (success)
    {
        std::vector<User const *> users = model_.getUsers();

        for (auto u : users)
        {
            assert(u);
            userList_->add(*u);
        }
    }
    else
    {
        append("Login failed: " + info, true);
    }
}

void LogUsersTab::connected(bool connected)
{
    if (!connected)
    {
        userList_->clear();
        append("Disconnected from server", true);
    }
}

void LogUsersTab::message(std::string const & msg)
{
    append(msg, true);
}

void LogUsersTab::userJoined(User const & user)
{
    userList_->add(user);
}

void LogUsersTab::userLeft(User const & user)
{
    userList_->remove(user.name());
}

void LogUsersTab::ring(std::string const & userName)
{
    append("ring from " + userName, true);
    Sound::beep();
}

int LogUsersTab::handle(int event)
{
    switch (event)
    {
    case FL_SHOW:
        labelcolor(FL_FOREGROUND_COLOR);
        Fl::focus(userList_);
        break;
    }
    return Fl_Tile::handle(event);
}

void LogUsersTab::append(std::string const & msg, bool interesting)
{
    logFile_.log(msg);

    text_->append(msg, 0);
    // make ChatTabs redraw header
    if (interesting && !visible() && labelcolor() != FL_RED)
    {
        labelcolor(FL_RED);
        iTabs_.redrawTabs();
    }

}

void LogUsersTab::downloadDone(std::string const & name, bool success)
{
    if (success)
    {
        append("download done: " + name, false);
    }
    else
    {
        append("download failed: " + name, true);
    }
}
