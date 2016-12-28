#include <QDebug>
#include <QMutexLocker>

#include "frame_buffer.h"


FrameBuffer::FrameBuffer(QMutex::RecursionMode recursion_mode, QObject *parent) :
    QObject(parent)
  , mutex_frame_buffer(new QMutex(recursion_mode))
{
    buffer_max_size=1000;

}

FrameBuffer::~FrameBuffer()
{
    delete mutex_frame_buffer;
}

void FrameBuffer::appendFrame(FrameBuffer::Frame frame)
{
    QMutexLocker ml(mutex_frame_buffer);

//    mutex_frame_buffer->lock();

    if(queue.size()<buffer_max_size)
        queue.append(frame);

    else {
        frame.ba_video.clear();

        queue.append(frame);

        emit frameSkipped(++frame_skipped);

        qCritical() << "frames skipped:" << frame_skipped << queue.size();
    }

//    mutex_frame_buffer->unlock();
}

void FrameBuffer::setMaxBufferSize(uint8_t size)
{
    QMutexLocker ml(mutex_frame_buffer);

    buffer_max_size=size;
}

void FrameBuffer::clear()
{
    QMutexLocker ml(mutex_frame_buffer);

    queue.clear();

    frame_skipped=0;
}
