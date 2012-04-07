#include "Bot.h"
#include "LobbyProtocol.h"

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

Bot::Bot(std::string const & name, std::string const & aiDll):
        name_(name),
        aiDll_(aiDll)
{
    battleStatus_.spectator(false);
    battleStatus_.allyTeam(1); // team 2
    battleStatus_.ready(true);
}

Bot::~Bot()
{
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

