#pragma once

#include <chrono>

class Sound
{
public:
    static void beep();

private:
    static std::chrono::time_point<std::chrono::steady_clock> timeLast_;

};
