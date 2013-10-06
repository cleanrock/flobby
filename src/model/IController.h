#pragma once

#include <string>

class IControllerEvent;

class IController
{
public:
    virtual void setIControllerEvent(IControllerEvent & iControllerEvent) = 0;
    virtual void connect(std::string const & host, std::string const & service) = 0;
    virtual void disconnect() = 0;
    virtual void send(std::string const& msg) = 0;
    virtual uint64_t lastSendTime() const = 0; // milliseconds since start
    virtual uint64_t timeNow() const = 0; // milliseconds since start
    virtual unsigned int startProcess(std::string const & cmd, bool logToFile = false) = 0; // e.g. "/usr/bin/spring script.txt"

protected:
    ~IController() {}

};
