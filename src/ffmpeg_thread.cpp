#include <QDebug>
#include <qcoreapplication.h>

#include "capture.h"
#include "frame_buffer.h"

#include "ffmpeg_thread.h"

FFMpegThread::FFMpegThread(QObject *parent)
    : QThread(parent)
{
    frame_buffer=new FrameBuffer(this);

    frame_buffer->setMaxSize(120);

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

    Frame::ptr frame;

    while(true) {
        if(frame_buffer->isEmpty())
            frame_buffer->wait();

        frame=frame_buffer->take();

        if(frame) {
            ffmpeg->appendFrame(frame);

            frame.reset();
        }

        QCoreApplication::processEvents();
    }
}
