// This file is part of flobby (GPL v2 or later), see the LICENSE file

#pragma once

#include <vector>
#include <iosfwd>
#include <string>


class Channel
{
public:
    Channel(std::istream & is); // CHANNEL content
    virtual ~Channel();

    std::string const & name() const;
    int userCount() const;
    std::string const & topic() const;

    bool operator==(Channel const & other) const;

private:
    std::string name_;
    int userCount_;
    std::string topic_;
};

typedef std::vector<Channel> Channels;

// inline methods
//
inline const std::string & Channel::name() const
{
    return name_;
}

inline int Channel::userCount() const
{
    return userCount_;
}

inline const std::string & Channel::topic() const
{
    return topic_;
}

inline bool Channel::operator==(Channel const & other) const
{
    return name_ == other.name_;
}
