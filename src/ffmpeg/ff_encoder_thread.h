#ifndef FF_ENCODER_THREAD_H
#define FF_ENCODER_THREAD_H

#include <QThread>

#include <atomic>

#include "ff_encoder.h"
#include "frame_buffer.h"

class FFEncoderThread : public QThread
{
    Q_OBJECT

public:
    FFEncoderThread(QObject *parent=0);
    ~FFEncoderThread();

    FrameBuffer::ptr frameBuffer();

    bool isWorking();

public slots:
    void setConfig(FFEncoder::Config cfg);
    void stopCoder();

private slots:
    void onStateChanged(bool state);

private:
    FrameBuffer::ptr frame_buffer;

    bool is_working;

    std::atomic <bool> running;

protected:
    void run();

signals:
    void sigSetConfig(FFEncoder::Config cfg);
    void sigStopCoder();

    void stats(FFEncoder::Stats s);
    void stateChanged(bool state);

    void errorString(QString err_string);
};

#endif // FF_ENCODER_THREAD_H
