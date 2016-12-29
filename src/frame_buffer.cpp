#include <QDebug>
#include <QMutexLocker>

#include "frame_buffer.h"


FrameBuffer::FrameBuffer(QMutex::RecursionMode recursion_mode, QObject *parent) :
    QObject(parent)
  , mutex_frame_buffer(new QMutex(recursion_mode))
{
    frame_skipped=0;

    drop_skipped=false;

    buffer_max_size=1000;
}

FrameBuffer::~FrameBuffer()
{
    delete mutex_frame_buffer;
}

void FrameBuffer::appendFrame(FrameBuffer::Frame frame)
{
    QMutexLocker ml(mutex_frame_buffer);

    if(queue.size()<buffer_max_size)
        queue.append(frame);

    else {
        if(!drop_skipped) {
            frame.ba_video.clear();

            queue.append(frame);
        }

        emit frameSkipped(++frame_skipped);
    }
}

void FrameBuffer::setMaxBufferSize(uint8_t size)
{
    QMutexLocker ml(mutex_frame_buffer);

    buffer_max_size=size;
}

void FrameBuffer::setDropSkipped(bool state)
{
    QMutexLocker ml(mutex_frame_buffer);

    drop_skipped=state;
}

void FrameBuffer::clear()
{
    QMutexLocker ml(mutex_frame_buffer);

    queue.clear();

    frame_skipped=0;
}
