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
#include <QMutexLocker>
#include <QQueue>
#include <QSize>

#include "event_waiting.h"
#include "frame.h"

class Signaler : public QObject
{
    Q_OBJECT

signals:
    void frameSkipped();
};

template <class T>
class FrameBuffer
{
public:
    typedef std::shared_ptr<FrameBuffer<T>> ptr;

    explicit FrameBuffer() {
        enabled=true;

        max_size=10;
    }

    ~FrameBuffer() {
        event.next();
    }

    static ptr make() {
        return ptr(new FrameBuffer<T>());
    }

    void append(T frame) {
        QMutexLocker ml(&mutex);

        if(!enabled)
            return;

        if(queue.size()<max_size) {
            queue.append(frame);

        } else {
            emit signaler.frameSkipped();
        }

        event.next();
    }

    T take() {
        QMutexLocker ml(&mutex);

        if(queue.isEmpty())
            return T();

        return queue.dequeue();
    }

    void wait() {
        {
            QMutexLocker ml(&mutex);

            if(!queue.isEmpty())
                return;
        }

        event.wait();
    }

    void setMaxSize(uint16_t size) {
        QMutexLocker ml(&mutex);

        max_size=size;
    }

    void setEnabled(bool value) {
        QMutexLocker ml(&mutex);

        enabled=value;

        queue.clear();

        event.next();
    }

    bool isEnabled() {
        QMutexLocker ml(&mutex);

        return enabled;
    }

    void clear() {
        QMutexLocker ml(&mutex);

        queue.clear();

        event.next();
    }

    bool isEmpty() {
        QMutexLocker ml(&mutex);

        return queue.isEmpty();
    }

    QPair <int, int> size() {
        QMutexLocker ml(&mutex);

        return qMakePair(queue.size(), max_size);
    }

    Signaler signaler;

private:
    QQueue <T> queue;
    QMutex mutex;
    uint16_t max_size;
    EventWaiting event;
    bool enabled;
};

#endif // FRAME_BUFFER_H
