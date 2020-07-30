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

#ifndef AUDIO_LEVEL_H
#define AUDIO_LEVEL_H

#include <QThread>

#include <atomic>

#include "frame_buffer.h"

class AudioLevel : public QThread
{
    Q_OBJECT

public:
    explicit AudioLevel(QObject *parent=0);
    ~AudioLevel();

    FrameBuffer<Frame::ptr>::ptr frameBuffer();

protected:
    void run();

private:
    FrameBuffer<Frame::ptr>::ptr frame_buffer;

    std::atomic <bool> running;

    int32_t level[8];

signals:
    void levels(qreal l, qreal r, qreal c, qreal lfe, qreal rl, qreal rr, qreal sl, qreal sr);
};

#endif // AUDIO_LEVEL_H
