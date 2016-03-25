// This file is part of flobby (GPL v2 or later), see the LICENSE file

#include "Battle.h"
#include "User.h"
#include "LobbyProtocol.h"

#include "log/Log.h"

#include <json/json.h>
#include <boost/lexical_cast.hpp>
#include <iostream>

Battle::Battle(std::istream & is): // battleId type natType founder IP port maxPlayers passworded rank mapHash {engineName} {engineVersion} {map} {title} {gameName}
        spectators_(0), // set to 1 below if replay
        locked_(false), // only set to true by UPDATEBATTLEINFO
        running_(false), // set by founder status
        modHash_(0)
{
    using namespace LobbyProtocol;

    std::string ex;

    extractWord(is, ex);
    id_ = boost::lexical_cast<int>(ex);

    extractWord(is, ex);
    replay_ = boost::lexical_cast<bool>(ex);

    extractWord(is, ex);
    natType_ = boost::lexical_cast<int>(ex);

    extractWord(is, founder_);

    extractWord(is, ip_);
    extractWord(is, port_);

    extractWord(is, ex);
    maxPlayers_ = boost::lexical_cast<int>(ex);

    extractWord(is, ex);
    passworded_ = boost::lexical_cast<bool>(ex);

    extractWord(is, ex);
    rank_ = boost::lexical_cast<int>(ex);

    extractWord(is, ex);
    mapHash_ = static_cast<unsigned int>( boost::lexical_cast<int64_t>(ex) );

    extractSentence(is, engineName_);
    extractSentence(is, engineVersion_);

    // separate engine version and branch
    std::istringstream iss(engineVersion_);
    iss >> engineVersion_;
    iss >> engineBranch_;

    engineVersionLong_ = engineVersion_;
    if (!engineBranch_.empty())
    {
        engineVersionLong_ += " (";
        engineVersionLong_ += engineBranch_;
        engineVersionLong_ += ")";
    }

    extractSentence(is, mapName_);
    extractSentence(is, title_);
    extractSentence(is, modName_);

    if (replay_)
    {
        spectators_ = 1;
    }
}

Battle::Battle(Json::Value & jv):
        locked_(false), // only set to true by UPDATEBATTLEINFO
        running_(false), // set by founder status
        modHash_(0)
{
    id_ = jv["BattleID"].asInt();

    replay_ = false;

    natType_ = 0;

    founder_ = jv["Founder"].asString();

    ip_ = jv["Ip"].asString();
    port_ = jv["Port"].asString();

    maxPlayers_ = jv["MaxPlayers"].asInt();

    passworded_ = !jv["Password"].asString().empty();

    rank_ = 0;

    mapHash_ = 0;

    engineName_ = "spring";
    engineVersion_ = jv["Engine"].asString();

    // separate engine version and branch
    std::istringstream iss(engineVersion_);
    iss >> engineVersion_;
    iss >> engineBranch_;

    engineVersionLong_ = engineVersion_;
    if (!engineBranch_.empty())
    {
        engineVersionLong_ += " (";
        engineVersionLong_ += engineBranch_;
        engineVersionLong_ += ")";
    }

    updateBattleUpdate(jv);
}

bool Battle::running(bool running)
{
    bool const changed = (running != running_);
    running_ = running;
    return changed;
}

Battle::~Battle()
{
}

void Battle::updateBattleInfo(std::istream & is)
{
    using namespace LobbyProtocol;

    std::string ex;

    extractWord(is, ex);
    spectators_ = boost::lexical_cast<int>(ex);

    extractWord(is, ex);
    locked_ = boost::lexical_cast<bool>(ex);

    extractWord(is, ex);
    mapHash_ = static_cast<unsigned int>( boost::lexical_cast<int64_t>(ex) );

    extractSentence(is, mapName_);
}

void Battle::updateBattleUpdate(Json::Value & jv)
{
    if (jv.isMember("Map")) mapName_ = jv["Map"].asString();
    if (jv.isMember("Title")) title_ = jv["Title"].asString();
    if (jv.isMember("Game")) modName_ = jv["Game"].asString();
    if (jv.isMember("SpectatorCount")) spectators_ = jv["SpectatorCount"].asInt();
}

void Battle::joined(User const & user)
{
    auto res = users_.insert( { user.name(), &user });
    if (!res.second)
    {
        LOG(WARNING) << "user " << user.name() << " already joined battle " << title();
    }
}

void Battle::left(User const & user)
{
    size_t res = users_.erase(user.name());
    if (res == 0)
    {
        LOG(WARNING) << "user " << user.name() << " was not in battle " << title();
    }
}

int Battle::playerCount() const
{
    int playerCount = 0;

    for (BattleUsers::value_type const& pairNamePtr: users_)
    {
        if (!pairNamePtr.second->status().bot())
        {
            ++playerCount;
        }
    }

    return playerCount;
}

int Battle::spectators() const
{
    int spectatorCount = spectators_;

    for (BattleUsers::value_type const& pairNamePtr: users_)
    {
        if (pairNamePtr.second->status().bot())
        {
            --spectatorCount;
        }
    }

    return spectatorCount;
}

void Battle::print(std::ostream & os) const
{
    os << "[Battle:" // TODO add more info
       << "id=" <<  id_ << ", "
       << "title=" <<  title_ << ", "
       << "mapName=" <<  mapName_ << ", "
       << "users=";
       for (BattleUsers::value_type const & pair: users_)
       {
           os << *pair.second << ",";
       }
       os << "]" << std::endl;
}

std::ostream& operator<<(std::ostream & os, Battle const & battle)
{
    battle.print(os);
    return os;
}

