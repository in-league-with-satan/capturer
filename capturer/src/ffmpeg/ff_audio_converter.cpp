/******************************************************************************

Copyright © 2018-2019 Andrey Cheprasov <ae.cheprasov@gmail.com>

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

#include "ff_audio_converter.h"

AudioConverter::AudioConverter()
{
    context=nullptr;

    free();
}

AudioConverter::~AudioConverter()
{
    free();
}

bool AudioConverter::isReady() const
{
    if(context)
        return true;

    return false;
}

bool AudioConverter::compareParams(uint64_t in_channels, size_t in_sample_size, int64_t in_sample_rate, uint64_t out_channels, size_t out_sample_size, int64_t out_sample_rate)
{
    return this->in_channels==in_channels
            && this->in_sample_size==in_sample_size
            && this->in_sample_rate==in_sample_rate
            && this->out_channels==out_channels
            && this->out_sample_size==out_sample_size
            && this->out_sample_rate==out_sample_rate;
}

bool AudioConverter::init(uint64_t in_channels_layout, int64_t in_sample_rate, int64_t in_sample_format, uint64_t out_channels_layout, int64_t out_sample_rate, int64_t out_sample_format)
{
    free();

    this->in_channels_layout=in_channels_layout;
    this->in_channels=av_get_channel_layout_nb_channels(in_channels_layout);
    this->in_sample_size=av_get_bytes_per_sample((AVSampleFormat)in_sample_format);
    this->in_sample_rate=in_sample_rate;
    this->in_sample_format=in_sample_format;

    this->out_channels_layout=out_channels_layout;
    this->out_channels=av_get_channel_layout_nb_channels(out_channels_layout);
    this->out_sample_size=av_get_bytes_per_sample((AVSampleFormat)out_sample_format);
    this->out_sample_rate=out_sample_rate;
    this->out_sample_format=out_sample_format;

    const int buf_size=128;

    char str_buf[buf_size]={ 0 };

    av_get_channel_layout_string(str_buf, buf_size, in_channels, in_channels_layout);

    QString str=QString("input: %1/%2/%3 ").arg(str_buf).arg(in_sample_rate).arg(av_get_sample_fmt_name((AVSampleFormat)in_sample_format));

    av_get_channel_layout_string(str_buf, buf_size, out_channels, out_channels_layout);

    str+=QString("output: %1/%2/%3").arg(str_buf).arg(out_sample_rate).arg(av_get_sample_fmt_name((AVSampleFormat)out_sample_format));

    qInfo().noquote() << str;

    context=swr_alloc();

    if(!context) {
        qCritical() << "swr_alloc err";
        return false;
    }

    int ret;

/*
    context=swr_alloc_set_opts(context,
                               out_channels_layout, (AVSampleFormat)out_sample_format, out_sample_rate,
                               in_channels_layout, (AVSampleFormat)in_sample_format, in_sample_rate,
                               0, nullptr);

    if(!context) {
        qCritical() << "swr_alloc err";
        return false;
    }
*/

    ret=av_opt_set_channel_layout(context, "in_channel_layout", in_channels_layout, 0);

    if(ret<0)
        goto err;

    ret=av_opt_set_channel_layout(context, "out_channel_layout", out_channels_layout, 0);

    if(ret<0)
        goto err;

    ret=av_opt_set_int(context, "in_sample_rate", in_sample_rate, 0);

    if(ret<0)
        goto err;

    ret=av_opt_set_int(context, "out_sample_rate", out_sample_rate, 0);

    if(ret<0)
        goto err;

    ret=av_opt_set_sample_fmt(context, "in_sample_fmt", (AVSampleFormat)in_sample_format, 0);

    if(ret<0)
        goto err;

    ret=av_opt_set_sample_fmt(context, "out_sample_fmt", (AVSampleFormat)out_sample_format, 0);

    if(ret<0)
        goto err;

    ret=swr_init(context);

    if(ret<0)
        goto err;

    return true;

err:

    qCritical() << "err" << ffErrorString(ret);

    free();

    return false;
}

bool AudioConverter::convert(void *src, size_t size, QByteArray *dst)
{
    if(!context) {
        qCritical() << "context null pointer";
        return false;
    }

    int64_t in_count=size/(in_sample_size*in_channels);

    int64_t out_count=av_rescale_rnd(swr_get_delay(context, in_sample_rate) + in_count,
                                     out_sample_rate, in_sample_rate, AV_ROUND_UP);

    dst->resize(out_count*out_channels*out_sample_size);


    char *ptr_dst=(char*)dst->constData();

    int out_count_res=swr_convert(context, (uint8_t**)&ptr_dst, out_count,
                                  (const uint8_t**)&src, in_count);

    if(out_count_res<out_count)
        dst->resize(out_count_res*out_channels*out_sample_size);

    return true;
}

bool AudioConverter::convert(QByteArray *src, QByteArray *dst)
{
    return convert((void*)src->constData(), src->size(), dst);
}

bool AudioConverter::convert(QByteArray *data)
{
    QByteArray ba_tmp;

    bool res=convert((void*)data->constData(), data->size(), &ba_tmp);

    (*data)=ba_tmp;

    return res;
}

AVFrame *AudioConverter::convert(void *src, size_t size)
{
    if(!context) {
        qCritical() << "context null pointer";
        return nullptr;
    }

    AVFrame *frame=av_frame_alloc();

    const int64_t in_samples=size/(in_sample_size*in_channels);

    frame->format=out_sample_format;
    frame->sample_rate=out_sample_rate;
    frame->channels=out_channels;
    frame->channel_layout=out_channels_layout;
    frame->nb_samples=swr_get_out_samples(context, in_samples);

    int ret=av_frame_get_buffer(frame, 0);

    if(ret<0) {
        qCritical() << "av_frame_get_buffer err:" << ffErrorString(ret);
        av_frame_free(&frame);
        return nullptr;
    }

    swr_convert(context, frame->data, frame->nb_samples, (const uint8_t**)&src, in_samples);

    return frame;
}

void AudioConverter::free()
{
    if(context) {
        swr_free(&context);

        context=nullptr;
    }

    in_channels_layout=0;
    in_channels=0;
    in_sample_size=0;
    in_sample_rate=0;
    in_sample_format=0;

    out_channels_layout=0;
    out_channels=0;
    out_sample_size=0;
    out_sample_rate=0;
    out_sample_format=0;
}

uint64_t AudioConverter::inChannelsLayout() const
{
    return in_channels_layout;
}

uint64_t AudioConverter::inChannels() const
{
    return in_channels;
}

int64_t AudioConverter::inSampleRate() const
{
    return in_sample_rate;
}

int64_t AudioConverter::inSampleFormat() const
{
    return in_sample_format;
}

uint64_t AudioConverter::outChannelsLayout() const
{
    return out_channels_layout;
}

uint64_t AudioConverter::outChannels() const
{
    return out_channels;
}

int64_t AudioConverter::outSampleRate() const
{
    return out_sample_rate;
}

int64_t AudioConverter::outSampleFormat() const
{
    return out_sample_format;
}

int AudioConverter::countOutSamples(int in_samples)
{
    if(!context) {
        qCritical() << "context null pointer";
        return -1;
    }

    return swr_get_out_samples(context, in_samples);
}

int AudioConverter::countOutputToInputSamples(int out_samples)
{
    int in_samples=0;
    int tmp;

    while(true) {
        tmp=swr_get_out_samples(context, in_samples);

        if(tmp==out_samples)
            return in_samples;

        in_samples++;

        if(in_samples>out_samples*0x400)
            return -1;
    }

    return 0;
}

