// This file is part of flobby (GPL v2 or later), see the LICENSE file

#pragma once

#include <string>
#include <ostream>

namespace Json {
    class Value;
}

class UserBattleStatus
{
public:
    UserBattleStatus(): val_(0) {}
    UserBattleStatus(std::string const & s); // integer string

    bool ready() const;
    void ready(bool ready);
    int team() const;
    void team(int index);
    int allyTeam() const;
    void allyTeam(int index);
    bool spectator() const;
    void spectator(bool spec);
    int handicap() const;
    int sync() const;
    void sync(int sync); // 0 = unknown, 1 = synced, 2 = unsynced
    int side() const;
    void side(int index);

    bool operator==(UserBattleStatus const & rh) const;
    bool operator!=(UserBattleStatus const & rh) const;

    void printWord(std::ostream & os) const;

private:
    int val_;
};

// inline methods
//
inline bool UserBattleStatus::operator==(UserBattleStatus const & rh) const
{
    return (val_ == rh.val_);
}

inline bool UserBattleStatus::operator!=(UserBattleStatus const & rh) const
{
    return !(*this == rh);
}

inline void UserBattleStatus::printWord(std::ostream & os) const
{
    os << val_;
}

// global inline functions
//
inline std::ostream& operator<<(std::ostream & os, UserBattleStatus const & ubs)
{
    ubs.printWord(os);
    return os;
}
