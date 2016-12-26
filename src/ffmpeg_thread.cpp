#include "capture.h"

#include "ffmpeg_thread.h"

FFMpegThread::FFMpegThread(DeckLinkCapture *decklink, QObject *parent) :
    QThread(parent)
  , decklink(decklink)
{
    setTerminationEnabled(true);

    start(QThread::NormalPriority);
}

FFMpegThread::~FFMpegThread()
{
}

void FFMpegThread::setConfig(FFMpeg::Config cfg)
{
    emit sigSetConfig(cfg);
}

void FFMpegThread::stopCoder()
{
    emit sigStopCoder();
}

void FFMpegThread::run()
{
    FFMpeg *ffmpeg=new FFMpeg();

    ffmpeg->moveToThread(this);

    connect(decklink, SIGNAL(frame(QByteArray,QSize,QByteArray)), ffmpeg, SLOT(appendFrame(QByteArray,QSize,QByteArray)), Qt::QueuedConnection);

    connect(this, SIGNAL(sigSetConfig(FFMpeg::Config)), ffmpeg, SLOT(setConfig(FFMpeg::Config)), Qt::QueuedConnection);
    connect(this, SIGNAL(sigStopCoder()), ffmpeg, SLOT(stopCoder()), Qt::QueuedConnection);

    exec();

    delete ffmpeg;
}
