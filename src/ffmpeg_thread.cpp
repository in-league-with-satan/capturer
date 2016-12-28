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


    QVector <FrameBuffer::Frame> frame;

    while(true) {
        {
            QMutexLocker ml(frame_buffer->mutex_frame_buffer);

            if(frame_buffer->queue.isEmpty())
                goto end;

            while(!frame_buffer->queue.isEmpty()) {
                frame.append(frame_buffer->queue.dequeue());

                if(frame.size()>=6)
                    break;
            }
        }


        for(int i=0, size=frame.size(); i<size; ++i)
            ffmpeg->appendFrame(&frame[i].ba_video, &frame[i].size_video, &frame[i].ba_audio);

        frame.clear();

end:
        QCoreApplication::processEvents();

        usleep(100);
    }
}
