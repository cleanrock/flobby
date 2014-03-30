// This file is part of flobby (GPL v2 or later), see the LICENSE file

// THIS FILE IS NOT USED

#include "Sound.h"

#include "log/Log.h"

#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>

static Mix_Chunk* sound = 0;

Sound::Sound()
{
    SDL_Init(SDL_INIT_AUDIO);

    int const frequency = MIX_DEFAULT_FREQUENCY;
    Uint16 const format = MIX_DEFAULT_FORMAT;
    int const channels = 1;
    int const chunksize = 2048; // very high to reduce CPU usage (perhaps pulseaudios fault)

    if (Mix_OpenAudio(frequency, format, channels, chunksize))
    {
        LOG(FATAL) << "Mix_OpenAudio failed";
    }

    sound = Mix_LoadWAV("/usr/share/sounds/KDE-Im-Low-Priority-Message.ogg");
    if (sound == 0)
    {
        LOG(FATAL) << "Mix_LoadWAV failed";
    }
}

Sound::~Sound()
{
    // disabled lines below are to make shutdown quick
    // Mix_CloseAudio();
    // Mix_FreeChunk(sound);
    Mix_Quit();
    // SDL_Quit();
}

void Sound::play()
{
    Mix_PlayChannel(-1, sound, 0);
}
