#include "Sound.h"

#include <cstdlib>

std::chrono::time_point<std::chrono::steady_clock> Sound::timeLast_ = std::chrono::steady_clock::now();

void Sound::beep()
{
    using namespace std::chrono;

    // limit to one beep per second
    auto const now = steady_clock::now();
    auto const diff = now - timeLast_;

    if (duration_cast<seconds>(diff).count() > 0)
    {
        std::system("xkbbell -v 100");
        timeLast_ = now;
    }

}
