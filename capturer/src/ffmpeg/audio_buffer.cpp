/******************************************************************************

Copyright © 2019-2020 Andrey Cheprasov <ae.cheprasov@gmail.com>

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

#include <QDebug>
#include "ff_tools.h"

#include "audio_buffer.h"

void AudioBuffer::init(int sample_fmt, int channels)
{
    data.clear();
    data_size=0;
    bytes_per_sample=av_get_bytes_per_sample((AVSampleFormat)sample_fmt);
    this->channels=channels;
}

void AudioBuffer::put(uint8_t *data, int size, int64_t pts, AVRational time_base)
{
    this->data.append({ QByteArray((char*)data, size), pts, time_base });
    data_size+=size;
}

AudioBuffer::AudioData AudioBuffer::get(int size)
{
    AudioData dres;
    AudioData dtmp;

    int size_available=0;

    while(!data.isEmpty()) {
        dtmp=data.takeFirst();

        if(dres.pts==AV_NOPTS_VALUE)
            dres.pts=av_rescale_q(dtmp.pts, dtmp.time_base, { 1, 48000 });

        size_available=std::min(dtmp.data.size(), size - dres.data.size());

        dres.data.append(dtmp.data.left(size_available));

        if(dtmp.data.size()>size_available) {
            dtmp.data.remove(0, size_available);

            if(dtmp.pts!=AV_NOPTS_VALUE)
                dtmp.pts+=av_rescale_q(size_available/(channels*bytes_per_sample), { 1, 48000 }, dtmp.time_base);

            data.prepend(dtmp);
        }

        if(dres.data.size()==size) {
            data_size-=dres.data.size();

            return dres;
        }
    }

    data_size-=dres.data.size();

    return dres;
}

AudioBuffer::AudioData AudioBuffer::getSamples(int samples_count)
{
    return get(samples_count*channels*bytes_per_sample);
}

int AudioBuffer::size() const
{
    return data_size;
}

int AudioBuffer::sizeSamples() const
{
    return data_size/(channels*bytes_per_sample);
}

