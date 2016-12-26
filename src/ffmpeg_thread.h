#ifndef FFMPEG_THREAD_H
#define FFMPEG_THREAD_H

#include <QThread>

#include "ffmpeg.h"

class DeckLinkCapture;

class FFMpegThread : public QThread
{
    Q_OBJECT

public:
    FFMpegThread(DeckLinkCapture *decklink, QObject *parent=0);
    ~FFMpegThread();

public slots:
    void setConfig(FFMpeg::Config cfg);
    void stopCoder();

protected:
    void run();

private:
    DeckLinkCapture *decklink;

signals:
    void sigSetConfig(FFMpeg::Config cfg);
    void sigStopCoder();

};

#endif // FFMPEG_THREAD_H
