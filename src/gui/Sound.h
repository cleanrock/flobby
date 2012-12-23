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
