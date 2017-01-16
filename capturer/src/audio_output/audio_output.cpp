#include "audio_output_thread.h"
#include "sdl2_audio_output_thread.h"
#include "pulse_audio_output_thread.h"

#include "audio_output.h"

AudioOutputInterface *newAudioOutput(QObject *parent)
{
#ifdef USE_PULSE_AUDIO
    return new PulseAudioOutputThread(parent);
#endif


#ifdef USE_SDL2
    return new Sdl2AudioOutputThread(parent);
#endif


    return new AudioOutputThread(parent);
}
