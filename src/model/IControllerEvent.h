#pragma once

#include <string>

struct IControllerEvent
{
    virtual void connected(bool connected) = 0;
    virtual void message(std::string const & msg) = 0;
    virtual void processDone(unsigned int id) = 0;
};
