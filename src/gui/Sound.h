#pragma once

#include <chrono>
#include <string>

class Sound
{
public:
    static void beep(char const * command = nullptr);

    static bool enable_;
    static std::string command_;

private:
    static std::chrono::time_point<std::chrono::steady_clock> timeLast_;

};
