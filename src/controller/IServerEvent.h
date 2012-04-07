#pragma once

#include <string>

struct IServerEvent
{
    virtual void connected(bool connected) = 0;
    virtual void message(std::string const & msg) = 0;
};
