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

#ifndef FRAME_BUFFER_H
#define FRAME_BUFFER_H

#include <QObject>
#include <QMutex>
#include <QQueue>
#include <QSize>

#include "event_waiting.h"
#include "frame.h"

class FrameBuffer : public QObject
{
    Q_OBJECT

public:
    typedef std::shared_ptr<FrameBuffer> ptr;

    explicit FrameBuffer(QObject *parent=0);
    ~FrameBuffer();

    static ptr make() {
        return ptr(new FrameBuffer());
    }

    void append(Frame::ptr frame);

    Frame::ptr take();

    void wait();

    void setMaxSize(uint16_t size);

    void setEnabled(bool value);
    bool isEnabled();

    void clear();

    bool isEmpty();

    QPair <int, int> size();

private:
    QQueue <Frame::ptr> queue;
    QMutex mutex;
    uint16_t max_size;
    EventWaiting event;
    bool enabled;

signals:
    void frameSkipped();
};

#endif // FRAME_BUFFER_H
