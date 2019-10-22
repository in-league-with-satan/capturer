/******************************************************************************

Copyright © 2019 Andrey Cheprasov <ae.cheprasov@gmail.com>

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

#ifndef AUDIO_BUFFER_H
#define AUDIO_BUFFER_H

#include <QByteArray>
#include <QList>

#include "ff_tools.h"

class AudioBuffer
{
public:
    struct AudioData {
        QByteArray data;
        int64_t pts=AV_NOPTS_VALUE;
        AVRational time_base={ 1, 48000 };
    };

    void init(int sample_fmt, int channels);

    void put(uint8_t *data, int size, int64_t pts, AVRational time_base);

    AudioData get(int size);

    AudioData getSamples(int samples_count);

    int size() const;

    int sizeSamples() const;

public:
    QList <AudioData> data;
    int channels=0;
    int bytes_per_sample=0;
    int64_t data_size=0;
};

#endif // AUDIO_BUFFER_H
