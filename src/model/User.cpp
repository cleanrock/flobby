// This file is part of flobby (GPL v2 or later), see the LICENSE file

#include "User.h"
#include "LobbyProtocol.h"
#include "Battle.h"
#include "log/Log.h"
#include <boost/lexical_cast.hpp>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <json/value.h>


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

User::User(Json::Value & jv):
    color_(0),
    joinedBattle_(-1)
{
    name_ = jv["Name"].asString();
    country_ = jv["Country"].asString();
    zkClientType_ = jv["LobbyVersion"].asString();
    if (zkClientType_.empty()) {
        LOG(WARNING)<< "empty LobbyVersion for user "<< name_;
        zkClientType_ = "empty";
    }
    // append Linux if bit 1 is set, see enum ClientTypes in ZKS code
    if (jv["ClientType"].asInt() & 0x2) {
        zkClientType_ += " Linux";
    }

    if (jv.isMember("AccountID")) zkAccountID_ = jv["AccountID"].asString();

    status_.bot(jv["IsBot"].asBool());
    status_.moderator(jv["IsAdmin"].asBool());

    updateUser(jv);
}

User::~User()
{
}

void User::updateUser(Json::Value& jv)
{
    if (jv.isMember("IsInGame")) status_.inGame(jv["IsInGame"].asBool());
    if (jv.isMember("IsAway")) status_.away(jv["IsAway"].asBool());
    if (jv.isMember("BattleID")) joinedBattle_ = jv["BattleID"].asInt();
}

void User::updateUserBattleStatus(Json::Value& jv)
{
    if (jv.isMember("AllyNumber")) battleStatus_.allyTeam(jv["AllyNumber"].asInt());
    if (jv.isMember("IsSpectator")) battleStatus_.spectator(jv["IsSpectator"].asBool());
    if (jv.isMember("Sync")) battleStatus_.sync(jv["Sync"].asInt());
    if (jv.isMember("TeamNumber")) battleStatus_.team(jv["TeamNumber"].asInt());
}

std::string const User::info() const
{
    std::ostringstream oss;
    oss << name_ << " ";
    oss << "country:" << country_ << " ";
    oss << "lobby:";

    if (!zkClientType_.empty())
    {
        oss << zkClientType_;
    }
    else
    {
        try
        {
            int const lobby = boost::lexical_cast<int>(cpu_); // throws bad_cast
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

