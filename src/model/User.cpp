#include "User.h"
#include "LobbyProtocol.h"
#include "Battle.h"

#include <iostream>
#include <stdexcept>


User::User(std::istream & is):
    joinedBattle_(0)
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

void User::leftBattle(Battle const & battle)
{
    if ( joinedBattle_ == 0 || joinedBattle_->id() != battle.id() )
    {
        throw std::runtime_error(__PRETTY_FUNCTION__);
    }
    joinedBattle_ = 0;
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

