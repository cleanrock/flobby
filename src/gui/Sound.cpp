// This file is part of flobby (GPL v2 or later), see the LICENSE file

#include "Sound.h"

#include <boost/chrono.hpp>
#include <cstdlib>

bool Sound::enable_ = true;
std::string Sound::command_;

static boost::chrono::time_point<boost::chrono::steady_clock> timeLast_ = boost::chrono::steady_clock::now();

void Sound::beep(char const * command)
{
    using namespace boost::chrono;

    if (enable_ || command)
    {
        // limit to one beep per second
        auto const now = steady_clock::now();
        auto const diff = now - timeLast_;

        std::string const cmd = command ? command : command_.c_str();
        if (command || duration_cast<seconds>(diff).count() > 0)
        {
            std::string const cmd2(cmd + " > /dev/null 2>&1 &");
            int const ret = std::system(cmd2.c_str());
            static_cast<void>(ret);
            timeLast_ = now;
        }
    }
}
