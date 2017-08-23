#ifndef FF_ENCODER_THREAD_MANAGER_H
#define FF_ENCODER_THREAD_MANAGER_H

#include <QThread>

#include <atomic>

#include "ff_encoder.h"
#include "frame_buffer.h"
#include "ff_encoder_thread.h"

class FFEncoderThreadManager : public QThread
{
    Q_OBJECT

public:
    FFEncoderThreadManager(QObject *parent=0);
    ~FFEncoderThreadManager();

    FrameBuffer::ptr frameBuffer();

    bool isWorking();

public slots:
    void setConfig(FFEncoder::Config cfg);
    void stopCoder();

private:
    FFEncoderThread *enc_a;
    FFEncoderThread *enc_b;

    FrameBuffer::ptr frame_buffer;

    std::atomic <bool> running;

protected:
    void timerEvent(QTimerEvent *event);

    void run();

signals:
    void sigSetConfig(FFEncoder::Config cfg);
    void sigStopCoder();

    void stats(FFEncoder::Stats s);
    void stateChanged(bool state);
};

#endif // FF_ENCODER_THREAD_MANAGER_H
