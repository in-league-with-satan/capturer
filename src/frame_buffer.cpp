#include <QDebug>
#include <QMutexLocker>

#include "frame_buffer.h"


FrameBuffer::FrameBuffer(QMutex::RecursionMode recursion_mode, QObject *parent)
    : QObject(parent)
    , mutex_frame_buffer(new QMutex(recursion_mode))
{
    enabled=true;

    buffer_max_size=10;
}

FrameBuffer::~FrameBuffer()
{
    delete mutex_frame_buffer;
}

void FrameBuffer::appendFrame(Frame::ptr frame)
{
    QMutexLocker ml(mutex_frame_buffer);

    if(!enabled)
        return;

    if(queue.size()<buffer_max_size) {
        queue.append(frame);

    } else {
        emit frameSkipped();
    }

    event.next();
}

void FrameBuffer::setMaxBufferSize(uint16_t size)
{
    QMutexLocker ml(mutex_frame_buffer);

    buffer_max_size=size;
}

void FrameBuffer::setEnabled(bool value)
{
    QMutexLocker ml(mutex_frame_buffer);

    enabled=value;

    queue.clear();

    event.next();
}

void FrameBuffer::clear()
{
    QMutexLocker ml(mutex_frame_buffer);

    queue.clear();

    event.next();
}

QPair <int, int> FrameBuffer::size()
{
    QMutexLocker ml(mutex_frame_buffer);

    return qMakePair(queue.size(), buffer_max_size);
}
