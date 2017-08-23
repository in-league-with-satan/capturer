#include <qcoreapplication.h>

#include "ff_encoder_thread_manager.h"

FFEncoderThreadManager::FFEncoderThreadManager(QObject *parent)
    : QThread(parent)
    , enc_a(nullptr)
    , enc_b(nullptr)
{
    frame_buffer=FrameBuffer::make();

    frame_buffer->setMaxSize(4);

    //

//    enc_a=new FFEncoderThread(true, this);

//    connect(this, SIGNAL(sigSetConfig(FFEncoder::Config)), enc_a, SLOT(setConfig(FFEncoder::Config)), Qt::QueuedConnection);
//    connect(this, SIGNAL(sigStopCoder()), enc_a, SLOT(stopCoder()), Qt::QueuedConnection);
//    connect(enc_a, SIGNAL(stats(FFEncoder::Stats)), SIGNAL(stats(FFEncoder::Stats)), Qt::QueuedConnection);
//    connect(enc_a, SIGNAL(stateChanged(bool)), SIGNAL(stateChanged(bool)), Qt::QueuedConnection);

//    //

//    enc_b=new FFEncoderThread(false, this);

//    connect(this, SIGNAL(sigSetConfig(FFEncoder::Config)), enc_b, SLOT(setConfig(FFEncoder::Config)), Qt::QueuedConnection);
//    connect(this, SIGNAL(sigStopCoder()), enc_b, SLOT(stopCoder()), Qt::QueuedConnection);
//    connect(enc_b, SIGNAL(stats(FFEncoder::Stats)), SIGNAL(stats(FFEncoder::Stats)), Qt::QueuedConnection);
//    connect(enc_b, SIGNAL(stateChanged(bool)), SIGNAL(stateChanged(bool)), Qt::QueuedConnection);

    //

//    startTimer(2);

    start(QThread::NormalPriority);
}

FFEncoderThreadManager::~FFEncoderThreadManager()
{
    running=false;

    while(isRunning()) {
        msleep(30);
    }
}

FrameBuffer::ptr FFEncoderThreadManager::frameBuffer()
{
    return frame_buffer;
}

bool FFEncoderThreadManager::isWorking()
{
    if(!enc_a)
        return false;

    return enc_a->isWorking();
}

void FFEncoderThreadManager::setConfig(FFEncoder::Config cfg)
{
    emit sigSetConfig(cfg);

//    frame_buffer->clear();
//    frame_buffer->setEnabled(true);
}

void FFEncoderThreadManager::stopCoder()
{
    emit sigStopCoder();
}

void FFEncoderThreadManager::timerEvent(QTimerEvent *event)
{
    Frame::ptr frame=frame_buffer->take();

    if(frame) {
//            frame_buffer_a->append(frame);
//            frame_buffer_b->append(frame);

        frame.reset();
    }
}

void FFEncoderThreadManager::run()
{
    enc_a=new FFEncoderThread(true);
    enc_a->moveToThread(this);

    connect(this, SIGNAL(sigSetConfig(FFEncoder::Config)), enc_a, SLOT(setConfig(FFEncoder::Config)), Qt::QueuedConnection);
    connect(this, SIGNAL(sigStopCoder()), enc_a, SLOT(stopCoder()), Qt::QueuedConnection);
    connect(enc_a, SIGNAL(stats(FFEncoder::Stats)), SIGNAL(stats(FFEncoder::Stats)), Qt::QueuedConnection);
    connect(enc_a, SIGNAL(stateChanged(bool)), SIGNAL(stateChanged(bool)), Qt::QueuedConnection);

    //

    enc_b=new FFEncoderThread(false);
    enc_b->moveToThread(this);

    connect(this, SIGNAL(sigSetConfig(FFEncoder::Config)), enc_b, SLOT(setConfig(FFEncoder::Config)), Qt::QueuedConnection);
    connect(this, SIGNAL(sigStopCoder()), enc_b, SLOT(stopCoder()), Qt::QueuedConnection);
    connect(enc_b, SIGNAL(stats(FFEncoder::Stats)), SIGNAL(stats(FFEncoder::Stats)), Qt::QueuedConnection);
    connect(enc_b, SIGNAL(stateChanged(bool)), SIGNAL(stateChanged(bool)), Qt::QueuedConnection);

    //

//    exec();

    FrameBuffer::ptr frame_buffer_a=enc_a->frameBuffer();
    FrameBuffer::ptr frame_buffer_b=enc_b->frameBuffer();

    Frame::ptr frame;

    running=true;

    while(running) {
        frame_buffer->wait();

        frame=frame_buffer->take();

        if(frame) {
            frame_buffer_a->append(frame);
            frame_buffer_b->append(frame);

            frame.reset();
        }

        QCoreApplication::processEvents();
    }

    enc_a->deleteLater();
    enc_b->deleteLater();
}
