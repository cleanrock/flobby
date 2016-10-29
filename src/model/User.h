// This file is part of flobby (GPL v2 or later), see the LICENSE file

#pragma once

#include "UserStatus.h"
#include "UserBattleStatus.h"

#include <iosfwd>
#include <string>

class Battle;
namespace Json {
    class Value;
}

class User
{
public:
    User(std::istream & is); // ADDUSER content
    User(Json::Value& jv); // User content
    virtual ~User();

    std::pair<bool,int> updateUser(Json::Value& jv);
    void updateUserBattleStatus(Json::Value& jv); // UpdateUserBattleStatus content

    std::string const & name() const;
    std::string const & country() const;
    std::string const & cpu() const;

    std::string const info() const; // name, country, lobby
    std::string const zkAccountID() const { return zkAccountID_; }

    int color() const;
    void color(int color);

    UserStatus const & status() const;
    void status(UserStatus const& status);

    UserBattleStatus const & battleStatus() const;
    void battleStatus(UserBattleStatus const& battleStatus);

    void joinedBattle(Battle const& battle);
    int joinedBattle() const;
    void leftBattle(Battle const& battle);

    bool operator==(User const& other) const;
    bool operator!=(User const& other) const;
    void print(std::ostream & os) const;

private:
    friend class Model; // TODO remove

    std::string name_;
    std::string country_;
    std::string cpu_;
    std::string zkClientType_;
    std::string zkAccountID_;
    int color_; // 0x00BBGGRR
    UserStatus status_;
    UserBattleStatus battleStatus_;
    int joinedBattle_;
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

inline int User::joinedBattle() const
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

