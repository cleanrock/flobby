#pragma once

#include <string>

class IChat
{
public:
    virtual void openPrivateChat(std::string const & userName) = 0;
    virtual void openChannelChat(std::string const & channelName) = 0;

protected:
    ~IChat() {}
};
