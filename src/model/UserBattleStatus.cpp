// This file is part of flobby (GPL v2 or later), see the LICENSE file

#include "UserBattleStatus.h"

#include <boost/lexical_cast.hpp>

UserBattleStatus::UserBattleStatus(std::string const & s)
{
    val_ = boost::lexical_cast<int>(s); // throws boost::bad_lexical_cast on failure
}

bool UserBattleStatus::ready() const
{
    return (val_ & 0x2); // bit 1
}

void UserBattleStatus::ready(bool ready)
{
    if (ready)
    {
        val_ |= 0x02;
    }
    else
    {
        val_ &= ~0x02;

    }
}

int UserBattleStatus::team() const
{
    return ((val_ & 0x3C) >> 2); // bit 2-5
}

void UserBattleStatus::team(int index)
{
    assert(index >= 0 && index < 16);
    int const shifted = index << 2;
    val_ &= ~0x3C;
    val_ |= shifted;
}

int UserBattleStatus::allyTeam() const
{
    return ((val_ & 0x3C0) >> 6); // bit 6-9
}

void UserBattleStatus::allyTeam(int index)
{
    assert(index >= 0 && index < 16);
    int const shifted = index << 6;
    val_ &= ~0x3C0;
    val_ |= shifted;
}

bool UserBattleStatus::spectator() const
{
    return (val_ & 0x400) == 0; // bit 10
}

void UserBattleStatus::spectator(bool spec)
{
    if (spec)
    {
        val_ &= ~0x400;
    }
    else
    {
        val_ |= 0x400;
    }
}

int UserBattleStatus::handicap() const
{
    return ((val_ & 0x3F800) >> 11); // bit 11-17
}

int UserBattleStatus::sync() const
{
    return ((val_ & 0xC00000) >> 22); // bit 22-23
}

void UserBattleStatus::sync(int sync)
{
    assert(sync >= 0 && sync <= 2);
    int const shifted = sync << 22;
    val_ &= ~0xC00000;
    val_ |= shifted;
}

int UserBattleStatus::side() const
{
    return ((val_ & 0x0F000000) >> 24); // bit 24-27
}

void UserBattleStatus::side(int index)
{
    assert(index >= 0 && index < 16);
    int const shifted = index << 24;
    val_ &= ~0x0F000000;
    val_ |= shifted;
}

