// This file is part of flobby (GPL v2 or later), see the LICENSE file

#pragma once

#include <map>
#include <vector>
#include <string>

class Model;

class ServerCommand
{
public:
    static void init(Model& model);
    static std::string process(std::string const& str);

    virtual std::string description() =0;
    virtual std::string process(std::vector<std::string> const& args) =0;

protected:
    static Model* model_;

    typedef std::map<std::string, ServerCommand*> Commands;
    static Commands commmands_;

    std::string name_;

    ServerCommand(std::string const& name)
        : name_(name)
    {}
    virtual ~ServerCommand() {}

};
