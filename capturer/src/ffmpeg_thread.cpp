#include <QDebug>
#include <QMutexLocker>
#include <qcoreapplication.h>

#include "capture.h"
#include "frame_buffer.h"

#include "ffmpeg_thread.h"

FFMpegThread::FFMpegThread(QObject *parent)
    : QThread(parent)
{
    frame_buffer=new FrameBuffer(QMutex::Recursive, this);

    frame_buffer->setMaxBufferSize(120);

    frame_buffer->setDropSkipped(true);

    frame_buffer->setEnabled(false);

    is_working=false;

    setTerminationEnabled();

    start(QThread::NormalPriority);
    // start(QThread::HighPriority);
    // start(QThread::HighestPriority);
    // start(QThread::TimeCriticalPriority);
}

FFMpegThread::~FFMpegThread()
{
    terminate();

    delete frame_buffer;
}

FrameBuffer *FFMpegThread::frameBuffer()
{
    return frame_buffer;
}

bool FFMpegThread::isWorking()
{
    return is_working;
}

void FFMpegThread::setConfig(FFMpeg::Config cfg)
{
    emit sigSetConfig(cfg);

    frame_buffer->clear();
    frame_buffer->setEnabled(true);

    is_working=true;
}

void FFMpegThread::stopCoder()
{
    emit sigStopCoder();

    frame_buffer->clear();
    frame_buffer->setEnabled(false);

    is_working=false;
}

void FFMpegThread::run()
{
    FFMpeg *ffmpeg=new FFMpeg();

    ffmpeg->moveToThread(this);

    connect(this, SIGNAL(sigSetConfig(FFMpeg::Config)), ffmpeg, SLOT(setConfig(FFMpeg::Config)), Qt::QueuedConnection);
    connect(this, SIGNAL(sigStopCoder()), ffmpeg, SLOT(stopCoder()), Qt::QueuedConnection);
    connect(ffmpeg, SIGNAL(stats(FFMpeg::Stats)), SIGNAL(stats(FFMpeg::Stats)), Qt::QueuedConnection);

    FrameBuffer::Frame frame;

    bool queue_is_empty=true;

    while(true) {
        frame_buffer->event.wait();

begin:

        {
            QMutexLocker ml(frame_buffer->mutex_frame_buffer);

            if(frame_buffer->queue.isEmpty())
                goto end;

            frame=frame_buffer->queue.dequeue();

            queue_is_empty=frame_buffer->queue.isEmpty();
        }

        ffmpeg->appendFrame(&frame.ba_video, &frame.size_video, &frame.ba_audio);

        if(!queue_is_empty)
            goto begin;

end:

        QCoreApplication::processEvents();
    }
}
