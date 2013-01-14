#pragma once

#include "UserBattleStatus.h"

#include <iosfwd>
#include <string>

class Battle;


class Bot
{
public:
    Bot(std::istream & is); // ADDBOT content excluding initial battleId
    Bot(std::string const & name, std::string const & aiDll); // used when adding bot to battle
    virtual ~Bot();

    std::string const & name() const;
    std::string const & owner() const;

    UserBattleStatus const & battleStatus() const;
    void battleStatus(UserBattleStatus const & battleStatus);

    int color() const;
    void color(int color);

    std::string const & aiDll() const;

    bool operator==(Bot const & other) const;
    void print(std::ostream & os) const;

private:
    std::string name_;
    std::string owner_;
    UserBattleStatus battleStatus_;
    int color_;
    std::string aiDll_;
};

// inline methods
//
inline const std::string & Bot::name() const
{
    return name_;
}

inline const std::string & Bot::owner() const
{
    return owner_;
}

inline UserBattleStatus const & Bot::battleStatus() const
{
    return battleStatus_;
}

inline void Bot::battleStatus(UserBattleStatus const & battleStatus)
{
    battleStatus_ = battleStatus;
}

inline int Bot::color() const
{
    return color_;
}

inline void Bot::color(int color)
{
    color_ = color;
}

inline const std::string & Bot::aiDll() const
{
    return aiDll_;
}

inline bool Bot::operator==(Bot const & other) const
{
    return name_ == other.name_;
}

// global functions
//
std::ostream& operator<<(std::ostream & os, Bot const & bot);

