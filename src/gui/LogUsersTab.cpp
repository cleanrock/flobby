#include "LogUsersTab.h"
#include "TextDisplay.h"
#include "StringTable.h"
#include "Prefs.h"
#include "ITabs.h"
#include "IChat.h"
#include "PopupMenu.h"

#include "model/Model.h"

#include <FL/fl_ask.H>
#include <boost/bind.hpp>

static char const * PrefServerMessagesSplitH = "ServerMessagesSplitH";

LogUsersTab::LogUsersTab(int x, int y, int w, int h,
                               ITabs& iTabs, IChat & iChat, Model & model):
    Fl_Tile(x,y,w,h, "Log && Users"),
    iTabs_(iTabs),
    iChat_(iChat),
    model_(model),
    logFile_("messages")
{
    text_ = new TextDisplay(x, y, w/2, h);
    userList_ = new StringTable(x+w/2, y, w/2, h, "UserList",
            { "name", "status" });
    end();

    userList_->connectRowClicked( boost::bind(&LogUsersTab::userClicked, this, _1, _2) );
    userList_->connectRowDoubleClicked( boost::bind(&LogUsersTab::userDoubleClicked, this, _1, _2) );

    // model signals
    model_.connectConnected( boost::bind(&LogUsersTab::connected, this, _1) );
    model_.connectLoginResult( boost::bind(&LogUsersTab::loginResult, this, _1, _2) );
    model_.connectServerMsg( boost::bind(&LogUsersTab::message, this, _1) );
    model_.connectUserJoined( boost::bind(&LogUsersTab::userJoined, this, _1) );
    model_.connectUserChanged( boost::bind(&LogUsersTab::userChanged, this, _1) );
    model_.connectUserLeft( boost::bind(&LogUsersTab::userLeft, this, _1) );
    model_.connectRing( boost::bind(&LogUsersTab::ring, this, _1) );
    model_.connectDownloadDone( boost::bind(&LogUsersTab::downloadDone, this, _1) );
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

void LogUsersTab::loginResult(bool success, std::string const & info)
{
    if (success)
    {
        std::vector<User const *> users = model_.getUsers();

        for (auto u : users)
        {
            assert(u);
            userList_->addRow(makeRow(*u));
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
    userList_->addRow(makeRow(user));
}

void LogUsersTab::userChanged(User const & user)
{
    userList_->updateRow(makeRow(user));
}

void LogUsersTab::userLeft(User const & user)
{
    userList_->removeRow(user.name());
}

StringTableRow LogUsersTab::makeRow(User const & user)
{
    return StringTableRow( user.name(),
        {
            user.name(),
            statusString(user)
        } );
}

std::string LogUsersTab::statusString(User const & user)
{
    std::ostringstream oss;
    oss << (user.status().bot() ? "B" : "");
    oss << (user.joinedBattle() != 0 ? "J" : "");
    oss << (user.status().inGame() ? "G" : "");
    return oss.str();
}

void LogUsersTab::userClicked(int rowIndex, int button)
{
    if (button == FL_RIGHT_MOUSE)
    {
        StringTableRow const & row = userList_->getRow(static_cast<std::size_t>(rowIndex));

        User const & user = model_.getUser(row.id_);
        PopupMenu menu;
        menu.add("Open chat", 1);
        Battle const * battle = user.joinedBattle();
        if (battle != 0)
        {
            menu.add("Join same battle", 2);
        }

        if (menu.size() > 0)
        {
            int const id = menu.show();
            switch (id)
            {
            case 1:
                iChat_.openPrivateChat(user.name());
                break;
            case 2:
                if (battle->passworded())
                {
                    char const * password = fl_input("Enter battle password");
                    if (password != NULL)
                    {
                        model_.joinBattle(battle->id(), password);
                    }
                }
                else
                {
                    model_.joinBattle(battle->id());
                }
                break;
            }
        }
    }
}

void LogUsersTab::userDoubleClicked(int rowIndex, int button)
{
    StringTableRow const & row = userList_->getRow(static_cast<std::size_t>(rowIndex));
    iChat_.openPrivateChat(row.id_);
}

void LogUsersTab::ring(std::string const & userName)
{
    append("ring from " + userName, true);
    fl_beep();
}

int LogUsersTab::handle(int event)
{
    switch (event)
    {
    case FL_SHOW:
        labelcolor(FL_BLACK);
        Fl::focus(userList_);
        break;
    }
    return Fl_Tile::handle(event);
}

void LogUsersTab::append(std::string const & msg, bool interesting)
{
    logFile_.log(msg);

    text_->append(msg);
    // make ChatTabs redraw header
    if (interesting && !visible() && labelcolor() != FL_RED)
    {
        labelcolor(FL_RED);
        iTabs_.redrawTabs();
    }

}

void LogUsersTab::downloadDone(std::string const & name)
{
    append("download attempt done: " + name, true);
}
