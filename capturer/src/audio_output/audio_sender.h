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

#ifndef AUDIO_SENDER_H
#define AUDIO_SENDER_H

#include <QThread>

#include <atomic>

#include "frame_buffer.h"

class AudioSender : public QThread
{
    Q_OBJECT

public:
    explicit AudioSender(QObject *parent=0);
    ~AudioSender();

    FrameBuffer::ptr frameBuffer();

protected:
    void run();

private:
    FrameBuffer::ptr frame_buffer;

    std::atomic <bool> running;

};

#endif // AUDIO_RESENDER_H
