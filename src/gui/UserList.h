#pragma once

#include "StringTable.h"

#include <string>

class Model;
class ITabs;
class User;

class UserList: public StringTable
{
public:
    UserList(int x, int y, int w, int h, Model & model, ITabs & iTabs, bool savePrefs = false);

    void add(User const & user);
    void add(std::string const & userName);
    void remove(std::string const & userName);

    std::string completeUserName(std::string const& text);

private:
    Model & model_;
    ITabs & iTabs_;

    StringTableRow makeRow(User const & user);
    std::string statusString(User const & user);

    // StringTable signals
    void userClicked(int rowIndex, int button);
    void userDoubleClicked(int rowIndex, int button);

    // model signals
    void userChanged(User const & user);
};
