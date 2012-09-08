#pragma once

#include <string>

class IControllerEvent
{
public:
    virtual void connected(bool connected) = 0;
    virtual void message(std::string const & msg) = 0;
    virtual void processDone(std::pair<unsigned int, int> idRetPair) = 0;

protected:
    ~IControllerEvent() {}

};
