#include "ServerTab.h"
#include "TextDisplay2.h"
#include "ChatInput.h"
#include "UserList.h"
#include "Prefs.h"
#include "ITabs.h"
#include "PopupMenu.h"
#include "Sound.h"
#include "TextFunctions.h"

#include "model/Model.h"

#include <FL/fl_ask.H>
#include <boost/bind.hpp>
#include <boost/algorithm/string.hpp>

static char const * PrefServerMessagesSplitH = "ServerMessagesSplitH";

ServerTab::ServerTab(int x, int y, int w, int h,
                               ITabs& iTabs, Model & model):
    Fl_Tile(x,y,w,h, "Server"),
    iTabs_(iTabs),
    model_(model),
    logFile_("messages")
{
    // left side (text and input)
    int const leftW = 0.75*w;
    Fl_Group * left = new Fl_Group(x, y, leftW, h);
    int const ih = FL_NORMAL_SIZE*2; // input height
    text_ = new TextDisplay2(x, y, leftW, h-ih);
    input_ = new ChatInput(x, y+h-ih, leftW, ih);
    input_->connectText( boost::bind(&ServerTab::onInput, this, _1) );
    input_->connectComplete( boost::bind(&ServerTab::onComplete, this, _1, _2, _3) );
    left->resizable(text_);
    left->end();

    // right side (user list)
    int const rightW = w - leftW;
    userList_ = new UserList(x+leftW, y, rightW, h, model_, iTabs_, true); // we save the prefs for UserList

    end();

    text_->append("type /help to see built-in commands");
    text_->append("lines entered not starting with / will be sent to lobby server");

    // model signals
    model_.connectConnected( boost::bind(&ServerTab::connected, this, _1) );
    model_.connectServerInfo( boost::bind(&ServerTab::serverInfo, this, _1) );
    model_.connectLoginResult( boost::bind(&ServerTab::loginResult, this, _1, _2) );
    model_.connectServerMsg( boost::bind(&ServerTab::message, this, _1) );
    model_.connectUserJoined( boost::bind(&ServerTab::userJoined, this, _1) );
    model_.connectUserLeft( boost::bind(&ServerTab::userLeft, this, _1) );
    model_.connectRing( boost::bind(&ServerTab::ring, this, _1) );
    model_.connectDownloadDone( boost::bind(&ServerTab::downloadDone, this, _1, _2) );
}

ServerTab::~ServerTab()
{
    prefs().set(PrefServerMessagesSplitH, userList_->x());
}

void ServerTab::initTiles()
{
    int x;
    prefs().get(PrefServerMessagesSplitH, x, 0);
    if (x != 0)
    {
        position(userList_->x(), 0, x, 0);
    }
}

void ServerTab::serverInfo(ServerInfo const & si)
{
    std::ostringstream oss;
    oss << "ServerInfo: " << si;
    append(oss.str());
}

void ServerTab::loginResult(bool success, std::string const & info)
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

void ServerTab::connected(bool connected)
{
    if (!connected)
    {
        userList_->clear();
        append("Disconnected from server\n", true); // extra newline for clarity
    }
}

void ServerTab::message(std::string const & msg)
{
    append(msg, true);
}

void ServerTab::userJoined(User const & user)
{
    userList_->add(user);
}

void ServerTab::userLeft(User const & user)
{
    userList_->remove(user.name());
}

void ServerTab::ring(std::string const & userName)
{
    append("ring from " + userName, true);
    Sound::beep();
}

int ServerTab::handle(int event)
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

void ServerTab::append(std::string const & msg, bool interesting)
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

void ServerTab::downloadDone(std::string const & name, bool success)
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

void ServerTab::onInput(std::string const & text)
{
    std::string textTrimmed = text;
    boost::trim(textTrimmed);

    std::string result = model_.serverCommand(textTrimmed);
    if (!result.empty())
    {
        append("'" + textTrimmed + "' returned:\n" + result);
    }
}

void ServerTab::onComplete(std::string const & text, std::size_t pos, std::pair<std::string, std::size_t>& result)
{
    auto const pairWordPos = getLastWord(text, pos);

    std::string const userName = userList_->completeUserName(pairWordPos.first);

    if (!userName.empty())
    {
        result.first = text.substr(0, pairWordPos.second) + userName + text.substr(pos);
        result.second = pairWordPos.second + userName.length();
    }
}
