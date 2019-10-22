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

#include <limits>

#include "audio_buffer.h"
#include "ff_audio_converter.h"

#include "audio_mixer.h"

struct AudioMixerContext
{
    QMap <int, AudioBuffer> buffer_input;
    QMap <int, AudioConverter> converter;

    AudioBuffer buffer_output;

    int sample_rate;
    int sample_fmt;
    int channels;
    int src_amt;
    int src_amt_fact;
    uint32_t active_src_devices;
};

AudioMixer::AudioMixer()
{
    d=new AudioMixerContext();
}

AudioMixer::~AudioMixer()
{
    delete d;
}

void AudioMixer::init(int sample_rate, int sample_fmt, int channels, uint32_t active_src_devices)
{
    if(sample_fmt!=AV_SAMPLE_FMT_S16) {
        qCritical() << "not supported sample format!!1";
    }

    clear();

    d->sample_rate=sample_rate;
    d->sample_fmt=sample_fmt;
    d->channels=channels;
    d->active_src_devices=active_src_devices;

    d->src_amt=0;
    d->src_amt_fact=0;

    for(int i=31; i>=0; --i) {
        if(active_src_devices&(1 << i) && d->src_amt==0)
            d->src_amt=i + 1;

        if(active_src_devices&(1 << i))
            d->src_amt_fact++;
    }

    d->buffer_output.init(sample_fmt, channels);
}

void AudioMixer::processFrame(Frame::ptr frame)
{
    if(!frame->audio.data_ptr)
        return;

    if(!d->converter.contains(frame->device_index)) {
        d->converter[frame->device_index].init(av_get_default_channel_layout(frame->audio.channels), 48000, frame->audio.sample_size==16 ? AV_SAMPLE_FMT_S16 : AV_SAMPLE_FMT_S32,
                                               av_get_default_channel_layout(d->channels), d->sample_rate, d->sample_fmt);

        d->buffer_input[frame->device_index].init(AV_SAMPLE_FMT_S16, d->channels);
    }

    QByteArray ba_dst;

    d->converter[frame->device_index].convert(frame->audio.data_ptr, frame->audio.data_size, &ba_dst);
    d->buffer_input[frame->device_index].put((uint8_t*)ba_dst.constData(), ba_dst.size(), frame->audio.pts, frame->audio.time_base);


    mix();
}

void AudioMixer::clear()
{
    d->buffer_input.clear();
    d->converter.clear();
}

AudioBuffer::AudioData AudioMixer::get()
{
    return d->buffer_output.get(d->buffer_output.size());
}

void AudioMixer::mix()
{
    if(d->buffer_input.empty())
        return;

    const int sample_size=av_get_bytes_per_sample((AVSampleFormat)d->sample_fmt);

    int samples=std::numeric_limits<int>::max();

    for(int idx_input=0; idx_input<d->src_amt; ++idx_input)
        if(d->active_src_devices&(1 << idx_input))
            samples=std::min(samples, d->buffer_input[idx_input].size()/sample_size);

    if(!samples)
        return;

    QByteArray ba_dst;
    ba_dst.resize(samples*sample_size);

    QVector <AudioBuffer::AudioData> src_sig;
    src_sig.resize(d->src_amt);

    double sig_sum;

    int64_t pts=AV_NOPTS_VALUE;

    for(int idx_input=0; idx_input<d->src_amt; ++idx_input) {
        if(d->active_src_devices&(1 << idx_input)) {
            src_sig[idx_input]=d->buffer_input[idx_input].get(samples*sample_size);
        }

        if(idx_input==0)
            pts=src_sig[idx_input].pts;
    }

    for(int idx_sample=0; idx_sample<samples; ++idx_sample) {
        sig_sum=0.;

        for(int idx_input=0; idx_input<d->src_amt; ++idx_input) {
            if(d->active_src_devices&(1 << idx_input)) {
                sig_sum+=((int16_t*)src_sig[idx_input].data.constData())[idx_sample];
            }
        }

        ((int16_t*)ba_dst.constData())[idx_sample]=int16_t(sig_sum/(double)d->src_amt_fact);
    }

    d->buffer_output.put((uint8_t*)ba_dst.constData(), ba_dst.size(), pts, { 1, 48000 });


    if(d->active_src_devices&0x1) {
        int size=d->buffer_input[0].size();

        if(size==0) {
            for(int idx_input=1; idx_input<d->src_amt; ++idx_input) {
                if(d->active_src_devices&(1 << idx_input)) {
                    if(d->buffer_input[idx_input].size()>ba_dst.size()*4) {
                        qInfo() << "try sync" << ba_dst.size() << d->buffer_input[idx_input].size();
                        d->buffer_input[idx_input].get(d->buffer_input[idx_input].size());
                    }
                }
            }
        }
    }
}

