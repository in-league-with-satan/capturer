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

#ifndef AUDIO_SENDER_H
#define AUDIO_SENDER_H

#include <QThread>

#include <atomic>

#include "frame_buffer.h"

class AudioConverter;

class AudioSender : public QThread
{
    Q_OBJECT

public:
    explicit AudioSender(int dev_num, QObject *parent=0);
    ~AudioSender();

    FrameBuffer<Frame::ptr>::ptr frameBuffer();

protected:
    void run();

private:
    FrameBuffer<Frame::ptr>::ptr frame_buffer;

    AudioConverter *audio_converter;

    std::atomic <bool> running;

    int dev_num;
};

#endif // AUDIO_RESENDER_H
