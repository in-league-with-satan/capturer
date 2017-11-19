#ifndef AUDIO_SENDER_H
#define AUDIO_SENDER_H

#include <QThread>

#include <atomic>

#include "frame_buffer.h"

class AudioSender : public QThread
{
    Q_OBJECT

public:
    explicit AudioSender(QObject *parent=0);
    ~AudioSender();

    FrameBuffer::ptr frameBuffer();

protected:
    void run();

private:
    FrameBuffer::ptr frame_buffer;

    std::atomic <bool> running;

};

#endif // AUDIO_RESENDER_H
