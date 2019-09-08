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

#include "ff_tools.h"

#include "audio_buffer.h"

void AudioBuffer::init(int sample_fmt, int channels)
{
    ba_data.clear();
    bytes_per_sample=av_get_bytes_per_sample((AVSampleFormat)sample_fmt);
    this->channels=channels;
}

void AudioBuffer::put(uint8_t *data, int size)
{
    ba_data.append((char*)data, size);
}

QByteArray AudioBuffer::get(int size)
{
    const int size_available=std::min(ba_data.size(), size);
    QByteArray ba_result=ba_data.left(size_available);
    ba_data.remove(0, size_available);
    return ba_result;
}

QByteArray AudioBuffer::getSamples(int samples_count)
{
    return get(samples_count*channels*bytes_per_sample);
}

int AudioBuffer::size() const
{
    return ba_data.size();
}

int AudioBuffer::sizeSamples() const
{
    return ba_data.size()/(channels*bytes_per_sample);
}

