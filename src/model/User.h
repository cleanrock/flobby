#pragma once

#include "UserStatus.h"
#include "UserBattleStatus.h"

#include <iosfwd>
#include <string>

class Battle;


class User
{
public:
    User(std::istream & is); // ADDUSER content
    virtual ~User();

    std::string const & name() const;
    std::string const & country() const;
    std::string const & cpu() const;

    int color() const;
    void color(int color);

    UserStatus const & status() const;
    void status(UserStatus const & status);

    UserBattleStatus const & battleStatus() const;
    void battleStatus(UserBattleStatus const & battleStatus);

    void joinedBattle(Battle const & battle);
    Battle const * joinedBattle() const;
    void leftBattle(Battle const & battle);

    bool operator==(User const & other) const;
    bool operator!=(User const & other) const;
    void print(std::ostream & os) const;

private:
    friend class Model;

    std::string name_;
    std::string country_;
    std::string cpu_;
    int color_; // 0x00BBGGRR
    UserStatus status_;
    UserBattleStatus battleStatus_;

    Battle const * joinedBattle_; // TODO shared_ptr

};

// inline methods
//
inline const std::string & User::name() const
{
    return name_;
}

inline const std::string & User::country() const
{
    return country_;
}

inline const std::string & User::cpu() const
{
    return cpu_;
}

inline int User::color() const
{
    return color_;
}

inline void User::color(int color)
{
    color_ = color;
}

inline UserStatus const & User::status() const
{
    return status_;
}

inline void User::status(UserStatus const & status)
{
    status_ = status;
}

inline UserBattleStatus const & User::battleStatus() const
{
    return battleStatus_;
}

inline void User::battleStatus(UserBattleStatus const & battleStatus)
{
    battleStatus_ = battleStatus;
}

inline void User::joinedBattle(Battle const & battle)
{
    joinedBattle_ = &battle;
}

inline Battle const * User::joinedBattle() const
{
    return joinedBattle_;
}

inline bool User::operator==(User const & other) const
{
    return name_ == other.name_;
}

inline bool User::operator!=(User const & other) const
{
    return !(*this == other);
}

// global functions
//
std::ostream& operator<<(std::ostream & os, User const & user);

