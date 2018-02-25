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

#include <QDebug>
#include <QDateTime>

#include "audio_normalization.h"

const int16_t max_value=std::numeric_limits<int16_t>::max();

AudioNormalization::AudioNormalization(QObject *parent)
    : QObject(parent)
{
    update_time=1*1000;

    last_update_timestamp=0;

    gain_factor=1.;
    gain_change_step=.2;
    maximum_level_percentage=.9;

    max_level=0;
}

void AudioNormalization::proc(QByteArray *data, int channels, bool disable_auto_gain)
{
    if(channels<1 || channels>8)
        return;

    int16_t *ptr_data=(int16_t*)data->constData();

    if(!disable_auto_gain) {
        for(int pos=0, size=data->size()/2; pos<size; pos+=channels)
            for(int channel=0; channel<channels; ++channel)
                max_level=std::max(max_level, (int32_t)ptr_data[pos + channel]);

        //

        if(QDateTime::currentMSecsSinceEpoch() - last_update_timestamp>=update_time) {
            if(max_level>0) {
                if(max_level*gain_factor<max_value*maximum_level_percentage) {
                    emit gainFactorChanged(gain_factor+=gain_change_step);
                }
            }

            max_level=0;
            last_update_timestamp=QDateTime::currentMSecsSinceEpoch();
        }
    }

    //

    QByteArray ba_out;

    ba_out.resize(data->size());

    int16_t *ptr_out=(int16_t*)ba_out.constData();

    for(int pos=0, size=data->size()/2; pos<size; pos+=channels) {
        for(int channel=0; channel<channels; ++channel) {
            ptr_out[pos + channel]=ptr_data[pos + channel]*gain_factor;

            if(!disable_auto_gain) {
                if(abs(ptr_out[pos + channel])>=max_value*maximum_level_percentage) {
                    emit gainFactorChanged(gain_factor=gain_factor*(max_value*maximum_level_percentage/(double)abs(ptr_out[pos + channel])) - gain_change_step);

                    pos=0;
                    channel=0;
                }
            }
        }
    }

    (*data)=ba_out;
}

void AudioNormalization::setUpdateTime(uint16_t ms)
{
    update_time=ms;
}

void AudioNormalization::setGainChangeStep(double value)
{
    if(value<.001 || value>10.)
        return;

    gain_change_step=value;
}

void AudioNormalization::setMaximumLevelPercentage(double value)
{
    if(value<.001 || value>1.)
        return;

    maximum_level_percentage=value;
}

double AudioNormalization::gainFactor()
{
    return gain_factor;
}

void AudioNormalization::setGainFactor(double value)
{
    gain_factor=value;
}
