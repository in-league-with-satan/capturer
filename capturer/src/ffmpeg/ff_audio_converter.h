/******************************************************************************

Copyright © 2018-2019 Andrey Cheprasov <ae.cheprasov@gmail.com>

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

#ifndef FF_AUDIO_CONVERTER_H
#define FF_AUDIO_CONVERTER_H

#include <QByteArray>

#include "ff_tools.h"

class SwrContext;

class AudioConverter
{
public:
    AudioConverter();
    ~AudioConverter();

    bool isReady() const;

    bool compareParams(uint64_t in_channels, size_t in_sample_size, int64_t in_sample_rate,
                       uint64_t out_channels, size_t out_sample_size, int64_t out_sample_rate);

    bool init(uint64_t in_channels_layout, int64_t in_sample_rate, int64_t in_sample_format,
              uint64_t out_channels_layout, int64_t out_sample_rate, int64_t out_sample_format);

    bool convert(void *src, size_t size, QByteArray *dst);
    bool convert(QByteArray *src, QByteArray *dst);
    bool convert(QByteArray *data);
    AVFrame *convert(void *src, size_t size);

    void free();

    uint64_t inChannelsLayout() const;
    uint64_t inChannels() const;
    int64_t inSampleRate() const;
    int64_t inSampleFormat() const;

    uint64_t outChannelsLayout() const;
    uint64_t outChannels() const;
    int64_t outSampleRate() const;
    int64_t outSampleFormat() const;

    int countOutSamples(int in_samples);
    int countOutputToInputSamples(int out_samples);

private:
    SwrContext *context;

    uint64_t in_channels_layout;
    uint64_t in_channels;
    size_t in_sample_size;
    int64_t in_sample_rate;
    int64_t in_sample_format;

    uint64_t out_channels_layout;
    uint64_t out_channels;
    size_t out_sample_size;
    int64_t out_sample_rate;
    int64_t out_sample_format;
};

#endif // FF_AUDIO_CONVERTER_H
