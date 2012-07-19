#include "UserList.h"
#include "ITabs.h"
#include "PopupMenu.h"

#include "model/Model.h"

#include "FL/fl_ask.H"
#include <boost/bind.hpp>

UserList::UserList(int x, int y, int w, int h, Model & model, ITabs & iTabs, bool savePrefs):
    StringTable(x, y, w, h, "UserList", {"name", "status"}, savePrefs),
    model_(model),
    iTabs_(iTabs)
{
    connectRowClicked( boost::bind(&UserList::userClicked, this, _1, _2) );
    connectRowDoubleClicked( boost::bind(&UserList::userDoubleClicked, this, _1, _2) );

    model_.connectUserChanged( boost::bind(&UserList::userChanged, this, _1) );
}

void UserList::add(User const & user)
{
    addRow(makeRow(user));
}

void UserList::add(std::string const & userName)
{
    User const & user = model_.getUser(userName);
    add(user);
}

void UserList::remove(std::string const & userName)
{
    removeRow(userName);
}

StringTableRow UserList::makeRow(User const & user)
{
    return StringTableRow( user.name(),
        {
            user.name(),
            statusString(user)
        } );
}

std::string UserList::statusString(User const & user)
{
    std::ostringstream oss;
    oss << (user.status().bot() ? "B" : "");
    oss << (user.joinedBattle() != 0 ? "J" : "");
    oss << (user.status().inGame() ? "G" : "");
    return oss.str();
}

void UserList::userChanged(User const & user)
{
    try
    {
        updateRow(makeRow(user)); // throws if row not found
    }
    catch (std::runtime_error & e)
    {
        // ignore
    }
}

void UserList::userClicked(int rowIndex, int button)
{
    if (button == FL_RIGHT_MOUSE)
    {
        StringTableRow const & row = getRow(static_cast<std::size_t>(rowIndex));

        User const & user = model_.getUser(row.id_);
        PopupMenu menu;
        menu.add("Open chat", 1);
        Battle const * battle = user.joinedBattle();
        if (battle != 0)
        {
            std::string joinText = "Join " + battle->title();
            menu.add(joinText, 2);
        }

        if (menu.size() > 0)
        {
            int const id = menu.show();
            switch (id)
            {
            case 1:
                iTabs_.openPrivateChat(user.name());
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

void UserList::userDoubleClicked(int rowIndex, int button)
{
    StringTableRow const & row = getRow(static_cast<std::size_t>(rowIndex));
    iTabs_.openPrivateChat(row.id_);
}
