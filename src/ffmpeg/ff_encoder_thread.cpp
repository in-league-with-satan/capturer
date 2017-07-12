#include <QDebug>
#include <qcoreapplication.h>

#include "capture.h"

#include "ff_encoder_thread.h"

FFEncoderThread::FFEncoderThread(QObject *parent)
    : QThread(parent)
{
    frame_buffer=FrameBuffer::make();

    frame_buffer->setMaxSize(120);

    frame_buffer->setEnabled(false);

    is_working=false;

    start(QThread::NormalPriority);
    // start(QThread::HighPriority);
    // start(QThread::HighestPriority);
    // start(QThread::TimeCriticalPriority);
}

FFEncoderThread::~FFEncoderThread()
{
    running=false;

    frame_buffer->setEnabled(true);
    frame_buffer->append(nullptr);

    while(isRunning()) {
        msleep(30);
    }
}

FrameBuffer::ptr FFEncoderThread::frameBuffer()
{
    return frame_buffer;
}

bool FFEncoderThread::isWorking()
{
    return is_working;
}

void FFEncoderThread::setConfig(FFEncoder::Config cfg)
{
    emit sigSetConfig(cfg);

    frame_buffer->clear();
    frame_buffer->setEnabled(true);

    is_working=true;
}

void FFEncoderThread::stopCoder()
{
    emit sigStopCoder();

    frame_buffer->clear();
    frame_buffer->setEnabled(false);

    is_working=false;
}

void FFEncoderThread::run()
{
    FFEncoder *ffmpeg=new FFEncoder();

    ffmpeg->moveToThread(this);

    connect(this, SIGNAL(sigSetConfig(FFEncoder::Config)), ffmpeg, SLOT(setConfig(FFEncoder::Config)), Qt::QueuedConnection);
    connect(this, SIGNAL(sigStopCoder()), ffmpeg, SLOT(stopCoder()), Qt::QueuedConnection);
    connect(ffmpeg, SIGNAL(stats(FFEncoder::Stats)), SIGNAL(stats(FFEncoder::Stats)), Qt::QueuedConnection);
    connect(ffmpeg, SIGNAL(stateChanged(bool)), SIGNAL(stateChanged(bool)), Qt::QueuedConnection);
    connect(ffmpeg, SIGNAL(errorString(QString)), SIGNAL(errorString(QString)), Qt::QueuedConnection);

    Frame::ptr frame;

    running=true;

    while(running) {
        frame_buffer->wait();

        frame=frame_buffer->take();

        if(frame) {
            ffmpeg->appendFrame(frame);

            frame.reset();
        }

        QCoreApplication::processEvents();
    }
}
