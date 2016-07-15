// This file is part of flobby (GPL v2 or later), see the LICENSE file

#pragma once

#include <string>

class ITabs
{
public:
    virtual void openPrivateChat(std::string const & userName) = 0;
    virtual void openChannelChat(std::string const & channelName) = 0;
    virtual void redrawTabs() = 0;
    virtual int getSplitPos() = 0;
    virtual void setSplitPos(int x, void* ignore) = 0;

protected:
    ~ITabs() {}
};
