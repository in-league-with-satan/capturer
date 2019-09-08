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

class AudioBuffer
{
    QByteArray ba_data;
    int channels=0;
    int bytes_per_sample=0;

public:
    void init(int sample_fmt, int channels);

    void put(uint8_t *data, int size);

    QByteArray get(int size);

    QByteArray getSamples(int samples_count);

    int size() const;

    int sizeSamples() const;
};

#endif // AUDIO_BUFFER_H
