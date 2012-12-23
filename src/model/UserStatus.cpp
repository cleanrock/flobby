#include "UserStatus.h"

#include <boost/lexical_cast.hpp>

UserStatus::UserStatus(const std::string & s)
{
    val_ = boost::lexical_cast<int>(s); // throws boost::bad_lexical_cast on failure
}

bool UserStatus::inGame() const
{
    return (val_ & 0x1); // bit 0
}

void UserStatus::inGame(bool inGame)
{
    if (inGame)
    {
        val_ |= 0x1;
    }
    else
    {
        val_ &= ~0x1;
    }
}

bool UserStatus::away() const
{
    return (val_ & 0x2); // bit 1
}

void UserStatus::away(bool away)
{
    if (away)
    {
        val_ |= 0x2;
    }
    else
    {
        val_ &= ~0x2;
    }
}

int UserStatus::rank() const
{
    return ((val_ & 0x1c) >> 2); // bit 2-4
}



bool UserStatus::moderator() const
{
    return (val_ & 0x20); // bit 5
}

bool UserStatus::bot() const
{
    return (val_ & 0x40); // bit 6
}

