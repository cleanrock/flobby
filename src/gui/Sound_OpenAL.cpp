// THIS FILE IS NOT USED

#include "Sound.h"

#include "log/Log.h"

#include <AL/alut.h>

static ALuint buffer_;
static ALuint source_;

Sound::Sound()
{
    alutInit(0, 0);

//    buffer_ = alutCreateBufferFromFile("/usr/share/sounds/purple/receive.wav");
    buffer_ = alutCreateBufferWaveform(ALUT_WAVEFORM_SINE, 1000, 0, 0.2);
    ALenum err;
    err = alutGetError();
    if (err != ALUT_ERROR_NO_ERROR)
    {
        LOG(ERROR)<< alutGetErrorString(err);
    }

    alGenSources(1, &source_);

    alSourcei(source_, AL_BUFFER, buffer_);
}

Sound::~Sound()
{
    LOG(ERROR)<< "DTOR";
    alDeleteSources(1, &source_);
    alDeleteBuffers(1, &buffer_);
    alutExit();
}

void Sound::play()
{
    alSourcePlay(source_);
}
