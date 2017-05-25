#ifndef AUDIO_LEVEL_H
#define AUDIO_LEVEL_H

#include <QThread>

#include <atomic>

#include "frame_buffer.h"

class AudioLevel : public QThread
{
    Q_OBJECT

public:
    explicit AudioLevel(QObject *parent=0);
    ~AudioLevel();

    FrameBuffer::ptr frameBuffer();

protected:
    void run();

private:
    FrameBuffer::ptr frame_buffer;

    std::atomic <bool> running;

    int32_t level[8];

signals:
    void levels(qreal l, qreal r, qreal c, qreal lfe, qreal rl, qreal rr, qreal sl, qreal sr);
};

#endif // AUDIO_LEVEL_H
