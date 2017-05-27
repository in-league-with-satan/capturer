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

bool AudioConverter::init(uint64_t in_channels_layout, int64_t in_sample_rate, int64_t in_sample_format, uint64_t out_channels_layout, int64_t out_sample_rate, int64_t out_sample_format)
{
    free();

    qInfo() << "AudioConverter::init" << in_channels_layout << in_sample_rate << in_sample_format
            << out_channels_layout << out_sample_rate << out_sample_format;

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

    context=swr_alloc();

    if(!context) {
        qCritical() << "AudioConverter: swr_alloc err";
        return false;
    }

    int ret;

    ret=av_opt_set_channel_layout(context, "in_channel_layout", in_channels_layout, 0);

    if(ret<0)
        goto err;

    ret=av_opt_set_channel_layout(context, "out_channel_layout", out_channels_layout, 0);

    if(ret<0)
        goto err;

    ret=av_opt_set_int(context,"in_sample_rate", in_sample_rate, 0);

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

    qCritical() << "AudioConverter: err" << ffErrorString(ret);

    free();

    return false;
}

bool AudioConverter::convert(QByteArray *src, QByteArray *dst)
{
    if(!context) {
        qCritical() << "AudioConverter::convert: context null pointer";
        return false;
    }

    int64_t in_count=src->size()/(in_sample_size*in_channels);

    int64_t out_count=av_rescale_rnd(swr_get_delay(context, in_sample_rate) + in_count,
                                     out_sample_rate, in_sample_rate, AV_ROUND_UP);

    dst->resize(out_count*out_channels*out_sample_size);


    char *ptr_src=src->data();
    char *ptr_dst=dst->data();

    int out_count_res=swr_convert(context, (uint8_t**)&ptr_dst, out_count,
                                  (const uint8_t**)&ptr_src, in_count);

    if(out_count_res<out_count)
        dst->resize(out_count_res*out_channels*out_sample_size);

    return true;
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
