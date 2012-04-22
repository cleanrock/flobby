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
