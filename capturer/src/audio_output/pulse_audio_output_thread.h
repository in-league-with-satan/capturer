/******************************************************************************

Copyright © 2018 Andrey Cheprasov <ae.cheprasov@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as
published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

******************************************************************************/

#ifndef PULSE_AUDIO_OUTPUT_THREAD_H
#define PULSE_AUDIO_OUTPUT_THREAD_H

#include "audio_output_interface.h"

#ifdef USE_PULSE_AUDIO
#include <pulse/simple.h>
#endif

#include <QFile>

class PulseAudioOutputThread : public AudioOutputInterface
{
    Q_OBJECT

public:
    PulseAudioOutputThread(QObject *parent=0);
    ~PulseAudioOutputThread();

protected:
    virtual void run();

    void onInputFrameArrived(void *data, size_t size, int channels, int sample_size);

    void init();

#ifdef USE_PULSE_AUDIO
    pa_simple *s;
    pa_sample_spec ss;
#endif

    QFile f_src;
    QFile f_conv;
};

#endif // PULSE_AUDIO_OUTPUT_THREAD_H
