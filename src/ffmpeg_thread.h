#ifndef FFMPEG_THREAD_H
#define FFMPEG_THREAD_H

#include <QThread>

#include "ffmpeg.h"


class FrameBuffer;

class FFMpegThread : public QThread
{
    Q_OBJECT

public:
    FFMpegThread(QObject *parent=0);
    ~FFMpegThread();

    FrameBuffer *frameBuffer();

    bool isWorking();

public slots:
    void setConfig(FFMpeg::Config cfg);
    void stopCoder();

private:
    FrameBuffer *frame_buffer;

    bool is_working;

protected:
    void run();

signals:
    void sigSetConfig(FFMpeg::Config cfg);
    void sigStopCoder();

    void stats(FFMpeg::Stats s);
};

#endif // FFMPEG_THREAD_H
