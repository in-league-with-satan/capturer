/******************************************************************************

Copyright © 2018 Andrey Cheprasov <ae.cheprasov@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

******************************************************************************/

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
