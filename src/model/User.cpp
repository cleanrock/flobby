// This file is part of flobby (GPL v2 or later), see the LICENSE file

#include "User.h"
#include "LobbyProtocol.h"
#include "Battle.h"

#include <iostream>
#include <stdexcept>


User::User(std::istream & is):
    color_(0),
    joinedBattle_(-1)
{
    using namespace LobbyProtocol;

    extractWord(is, name_);

    extractWord(is, country_);

    extractWord(is, cpu_);

    // TODO extract accountID
}

User::~User()
{
}

void User::joinedBattle(Battle const& battle)
{
    joinedBattle_ = battle.id();
}


void User::leftBattle(Battle const & battle)
{
    if ( joinedBattle_ == -1 || joinedBattle_ != battle.id() )
    {
        throw std::runtime_error(__PRETTY_FUNCTION__);
    }
    joinedBattle_ = -1;
}

void User::print(std::ostream & os) const
{
    os << "[User:"
       << "name=" <<  name_ << ", "
       << "country=" <<  country_ << ", "
       << "cpu=" <<  cpu_
       << "]";
}

// global functions
//
std::ostream& operator<<(std::ostream & os, User const & user)
{
    user.print(os);
    return os;
}

