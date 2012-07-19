#pragma once

#include <string>

class ITabs
{
public:
    virtual void openPrivateChat(std::string const & userName) = 0;
    virtual void openChannelChat(std::string const & channelName) = 0;
    virtual void redrawTabs() = 0;

protected:
    ~ITabs() {}
};
