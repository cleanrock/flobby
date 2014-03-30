// This file is part of flobby (GPL v2 or later), see the LICENSE file

#pragma once

#include <string>

class IServerEvent
{
public:
    virtual void connected(bool connected) = 0;
    virtual void message(std::string const & msg) = 0;

protected:
    ~IServerEvent() {}

};
