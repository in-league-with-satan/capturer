#include <QDebug>
#include <QMutexLocker>

#include "frame_buffer.h"

FrameBuffer::FrameBuffer(QObject *parent)
    : QObject(parent)
{
    enabled=true;

    max_size=10;
}

FrameBuffer::~FrameBuffer()
{
    event.next();
}

void FrameBuffer::append(Frame::ptr frame)
{
    QMutexLocker ml(&mutex);

    if(!enabled)
        return;

    if(queue.size()<max_size) {
        queue.append(frame);

    } else {
        emit frameSkipped();
    }

    event.next();
}

Frame::ptr FrameBuffer::take()
{
    QMutexLocker ml(&mutex);

    if(queue.isEmpty())
        return Frame::ptr();

    return queue.dequeue();
}

void FrameBuffer::wait()
{
    {
        QMutexLocker ml(&mutex);

        if(!queue.isEmpty())
            return;
    }

    event.wait();
}

void FrameBuffer::setMaxSize(uint16_t size)
{
    QMutexLocker ml(&mutex);

    max_size=size;
}

void FrameBuffer::setEnabled(bool value)
{
    QMutexLocker ml(&mutex);

    enabled=value;

    queue.clear();

    event.next();
}

bool FrameBuffer::isEnabled()
{
    QMutexLocker ml(&mutex);

    return enabled;
}

void FrameBuffer::clear()
{
    QMutexLocker ml(&mutex);

    queue.clear();

    event.next();
}

bool FrameBuffer::isEmpty()
{
    QMutexLocker ml(&mutex);

    return queue.isEmpty();
}

QPair <int, int> FrameBuffer::size()
{
    QMutexLocker ml(&mutex);

    return qMakePair(queue.size(), max_size);
}
