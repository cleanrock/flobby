// This file is part of flobby (GPL v2 or later), see the LICENSE file

#pragma once

#include <string>

class Sound
{
public:
    static void beep(char const* command = nullptr);

    static bool enable_;
    static std::string command_;

private:

};
