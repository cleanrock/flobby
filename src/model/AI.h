#pragma once

#include <map>
#include <string>


class AI
{
public:
    AI();
    ~AI();

    std::string name_; // shortName
    std::map<std::string, std::string> info_;

};
