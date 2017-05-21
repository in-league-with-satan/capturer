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

    std::atomic <bool> running;

protected:
    void run();

signals:
    void sigSetConfig(FFEncoder::Config cfg);
    void sigStopCoder();

    void stats(FFEncoder::Stats s);
    void stateChanged(bool state);
};

#endif // FF_ENCODER_THREAD_H
