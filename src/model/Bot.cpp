// This file is part of flobby (GPL v2 or later), see the LICENSE file

#include "Bot.h"
#include "LobbyProtocol.h"

#include <json/json.h>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <stdexcept>


Bot::Bot(std::istream & is)
{
    using namespace LobbyProtocol;


    extractWord(is, name_);
    extractWord(is, owner_);

    std::string ex;
    extractWord(is, ex);
    battleStatus_ = UserBattleStatus(ex);

    extractWord(is, ex);
    color_ = boost::lexical_cast<int>(ex);

    extractSentence(is, aiDll_);
}

Bot::Bot(Json::Value & jv)
{
    name_ = jv["Name"].asString();
    owner_ = jv["Owner"].asString();

    battleStatus_.allyTeam(jv["AllyNumber"].asInt());
    if (jv.isMember("TeamNumber")) {
        battleStatus_.team(jv["TeamNumber"].asInt());
    }
    else {
        battleStatus_.team(0);
    }
    battleStatus_.spectator(false);
    battleStatus_.sync(1);

    color_ = 0;

    aiDll_ = jv["AiLib"].asString();
}

Bot::Bot(std::string const & name, std::string const & aiDll):
        name_(name),
        color_(0),
        aiDll_(aiDll)
{
    battleStatus_.spectator(false);
    battleStatus_.allyTeam(1); // team 2
    battleStatus_.ready(true);
}

Bot::~Bot()
{
}

void Bot::updateBotStatus(Json::Value & jv)
{
    if (jv.isMember("AllyNumber")) battleStatus_.allyTeam(jv["AllyNumber"].asInt());
    if (jv.isMember("TeamNumber")) battleStatus_.team(jv["TeamNumber"].asInt());
}

void Bot::print(std::ostream & os) const
{
    os << "[Bot:"
       << "name=" <<  name_ << ", "
       << "owner=" <<  owner_ << ", "
       << "aiDll=" <<  aiDll_
       << "]";
}

// global functions
//
std::ostream& operator<<(std::ostream & os, Bot const & bot)
{
    bot.print(os);
    return os;
}

