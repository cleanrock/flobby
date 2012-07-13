#include "Sound.h"

#include <cstdlib>

void Sound::beep()
{
    std::system("xkbbell -v 100");
}
