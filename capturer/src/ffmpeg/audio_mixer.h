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

#ifndef AUDIO_MIXER_H
#define AUDIO_MIXER_H

#include "frame.h"

struct AudioMixerContext;

class AudioMixer
{
public:
    AudioMixer();
    ~AudioMixer();

    void init(int sample_rate, int sample_fmt, int channels, uint32_t active_src_devices);

    void processFrame(Frame::ptr frame);

    void clear();

    QByteArray get();

private:
    void mix();

    AudioMixerContext *d;
};

#endif // AUDIO_MIXER_H
