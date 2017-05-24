#ifndef AUDIO_LEVEL_H
#define AUDIO_LEVEL_H

#include <QThread>

#include <atomic>

class FrameBuffer;

class AudioLevel : public QThread
{
    Q_OBJECT

public:
    explicit AudioLevel(QObject *parent=0);
    ~AudioLevel();

    FrameBuffer *frameBuffer();

protected:
    void run();

private:
    FrameBuffer *frame_buffer;

    std::atomic <bool> running;

    int16_t level[8];

signals:
    void levels(qint16 l, qint16 r, qint16 c, qint16 lfe, qint16 bl, qint16 br, qint16 sl, qint16 sr);
};

#endif // AUDIO_LEVEL_H
