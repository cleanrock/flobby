// This file is part of flobby (GPL v2 or later), see the LICENSE file

#include "User.h"
#include "LobbyProtocol.h"
#include "Battle.h"
#include "log/Log.h"
#include <boost/lexical_cast.hpp>
#include <sstream>
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

std::string const User::info() const
{
    std::ostringstream oss;
    oss << name_ << " ";
    oss << "country:" << country_ << " ";
    oss << "lobby:";
    try
    {
        int const lobby = boost::lexical_cast<int>(cpu_);
        switch (lobby)
        {
        case 6666:
            oss << "ZeroK-Springie";
            break;
        case 6667:
            oss << "ZeroK-Win";
            break;
        case 6668:
            oss << "ZeroK-Lin";
            break;
        case 7777:
            oss << "WebLobby-Win";
            break;
        case 7778:
            oss << "WebLobby-Lin";
            break;
        case 7779:
            oss << "WebLobby-Mac";
            break;
        case -1525630178:
            oss << "MUSLCE";
            break;
        case 0x464C4C: // FLL
            oss << "Flobby-Lin";
            break;
        default:
            oss << cpu_;
            break;
        }
    }
    catch (const boost::bad_lexical_cast &)
    {
        LOG(WARNING)<< "failed to convert '"<< cpu_<< "' to int";
        oss << cpu_;
    }

    return oss.str();
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

