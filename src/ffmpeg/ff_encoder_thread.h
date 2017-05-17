#ifndef FF_ENCODER_THREAD_H
#define FF_ENCODER_THREAD_H

#include <QThread>

#include "ff_encoder.h"


class FrameBuffer;

class FFEncoderThread : public QThread
{
    Q_OBJECT

public:
    FFEncoderThread(QObject *parent=0);
    ~FFEncoderThread();

    FrameBuffer *frameBuffer();

    bool isWorking();

public slots:
    void setConfig(FFEncoder::Config cfg);
    void stopCoder();

private:
    FrameBuffer *frame_buffer;

    bool is_working;

protected:
    void run();

signals:
    void sigSetConfig(FFEncoder::Config cfg);
    void sigStopCoder();

    void stats(FFEncoder::Stats s);
};

#endif // FF_ENCODER_THREAD_H