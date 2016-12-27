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

    start(QThread::NormalPriority);
}

FFMpegThread::~FFMpegThread()
{
    delete frame_buffer;
}

FrameBuffer *FFMpegThread::frameBuffer()
{
    return frame_buffer;
}

void FFMpegThread::setConfig(FFMpeg::Config cfg)
{
    emit sigSetConfig(cfg);

    frame_buffer->mutex_frame_buffer->lock();

    frame_buffer->clear();

    frame_buffer->mutex_frame_buffer->unlock();
}

void FFMpegThread::stopCoder()
{
    emit sigStopCoder();
}

void FFMpegThread::run()
{
    FFMpeg *ffmpeg=new FFMpeg();

    ffmpeg->moveToThread(this);

    connect(this, SIGNAL(sigSetConfig(FFMpeg::Config)), ffmpeg, SLOT(setConfig(FFMpeg::Config)), Qt::QueuedConnection);
    connect(this, SIGNAL(sigStopCoder()), ffmpeg, SLOT(stopCoder()), Qt::QueuedConnection);


    FrameBuffer::Frame frame;

    while(true) {
        {
            QMutexLocker ml(frame_buffer->mutex_frame_buffer);

//            frame_buffer->mutex_frame_buffer->lock();

            if(frame_buffer->queue.isEmpty()) {
//                frame_buffer->mutex_frame_buffer->unlock();
                goto end;
            }

            frame=frame_buffer->queue.dequeue();

//            frame_buffer->mutex_frame_buffer->unlock();
        }

        ffmpeg->appendFrame(frame.ba_video, frame.size_video, frame.ba_audio);

end:
        QCoreApplication::processEvents();

        usleep(100);
    }
}
