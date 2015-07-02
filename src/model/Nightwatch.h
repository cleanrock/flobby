// This file is part of flobby (GPL v2 or later), see the LICENSE file

#pragma once

#include <string>

struct NightwatchPm
{
    NightwatchPm(): valid_(false) {}
    bool valid_;
    std::string channel_;
    std::string user_;
    std::string time_;
    std::string text_;
};

NightwatchPm checkNightwatchPm(std::string const & msg);
