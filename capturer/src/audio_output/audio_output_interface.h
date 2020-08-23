/******************************************************************************

Copyright © 2018 Andrey Cheprasov <ae.cheprasov@gmail.com>

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

#ifndef AUDIO_OUTPUT_INTERFACE_H
#define AUDIO_OUTPUT_INTERFACE_H

#include <QThread>

#include <atomic>

#include "frame_buffer.h"

class AudioConverter;

class AudioOutputInterface : public QThread
{
    Q_OBJECT

public:
    AudioOutputInterface(QObject *parent=0);
    ~AudioOutputInterface();

    FrameBuffer<Frame::ptr>::ptr frameBuffer();

protected:
    QByteArray convert(void *data, size_t size, const int in_channels, int in_sample_size, int out_channels);

    FrameBuffer<Frame::ptr>::ptr frame_buffer;

    std::atomic <bool> running;

    AudioConverter *audio_converter;
};

#endif // AUDIO_OUTPUT_THREAD_H
