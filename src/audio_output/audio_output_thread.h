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

#ifndef AUDIO_OUTPUT_THREAD_H
#define AUDIO_OUTPUT_THREAD_H

#include <QAudioFormat>

#include "audio_output_interface.h"
#include "audio_io_device.h"

class QAudioOutput;
class QIODevice;

class AudioOutputThread : public AudioOutputInterface
{
    Q_OBJECT

public:
    AudioOutputThread(QObject *parent=0);
    ~AudioOutputThread();

protected:
    virtual void run();

private:
    QAudioOutput *audio_output;

    AudioIODevice dev_audio_output;

    QAudioFormat audio_format;

};

#endif // AUDIO_OUTPUT_THREAD_H
