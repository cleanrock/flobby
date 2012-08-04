#include "Sound.h"

#include <cstdlib>

bool Sound::enable_ = true;
std::string Sound::command_;

std::chrono::time_point<std::chrono::steady_clock> Sound::timeLast_ = std::chrono::steady_clock::now();

void Sound::beep(char const * command)
{
    using namespace std::chrono;

    if (enable_ || command)
    {
        // limit to one beep per second
        auto const now = steady_clock::now();
        auto const diff = now - timeLast_;

        std::string const cmd = command ? command : command_.c_str();
        if (command || duration_cast<seconds>(diff).count() > 0)
        {
            std::string const cmd2(cmd + " >/dev/null &");
            int const ret = std::system(cmd2.c_str());
            timeLast_ = now;
        }
    }
}
