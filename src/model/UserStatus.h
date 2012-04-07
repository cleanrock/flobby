#pragma once

#include <ostream>
#include <string>

class UserStatus
{
public:
    UserStatus(): val_(0) {}
    UserStatus(std::string const & s); // integer string in CLIENTSTATUS

    bool inGame() const;
    void inGame(bool inGame);
    bool away() const;
    int rank() const;
    bool moderator() const;
    bool bot() const;

    bool operator==(UserStatus const & rh) const;
    bool operator!=(UserStatus const & rh) const;

    void printWord(std::ostream & os) const;

private:
    int val_;
};

// inline methods
//
inline bool UserStatus::operator==(UserStatus const & rh) const
{
    return (val_ == rh.val_);
}

inline bool UserStatus::operator!=(UserStatus const & rh) const
{
    return !(*this == rh);
}

inline void UserStatus::printWord(std::ostream & os) const
{
    os << val_;
}

// global inline functions
//
inline std::ostream& operator<<(std::ostream & os, UserStatus const & us)
{
    us.printWord(os);
    return os;
}
