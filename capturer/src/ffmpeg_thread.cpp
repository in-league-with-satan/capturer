#include <QMutexLocker>
#include <qcoreapplication.h>

#include "capture.h"
#include "frame_buffer.h"

#include "ffmpeg_thread.h"

FFMpegThread::FFMpegThread(QObject *parent) :
    QThread(parent)
{
    setTerminationEnabled(true);

    frame_buffer=new FrameBuffer(QMutex::Recursive, this);

    frame_buffer->setMaxBufferSize(400);

    frame_buffer->setEnabled(false);

    is_working=false;

    // start(QThread::NormalPriority);
    // start(QThread::HighPriority);
    // start(QThread::HighestPriority);
    start(QThread::TimeCriticalPriority);
}

FFMpegThread::~FFMpegThread()
{
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

    while(true) {
        {
            QMutexLocker ml(frame_buffer->mutex_frame_buffer);

            if(frame_buffer->queue.isEmpty())
                goto end;

            frame=frame_buffer->queue.dequeue();
        }

        ffmpeg->appendFrame(&frame.ba_video, &frame.size_video, &frame.ba_audio);

        goto end2;

end:

        QCoreApplication::processEvents();
        usleep(100);

end2:
        ;
    }
}
