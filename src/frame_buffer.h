#ifndef FRAME_BUFFER_H
#define FRAME_BUFFER_H

#include <QObject>
#include <QMutex>
#include <QQueue>
#include <QSize>

class FrameBuffer : public QObject
{
    Q_OBJECT

public:
    explicit FrameBuffer(QMutex::RecursionMode recursion_mode=QMutex::Recursive, QObject *parent=0);
    ~FrameBuffer();

    struct Frame {
        QByteArray ba_video;
        QSize size_video;
        QByteArray ba_audio;
    };

    void appendFrame(const Frame &frame);

    void setMaxBufferSize(uint8_t size);

    void clear();

    QMutex *mutex_frame_buffer;

    size_t frame_skipped;

    uint8_t buffer_max_size;

    QQueue <Frame> queue;

signals:
    void frameSkipped(size_t size);
};

#endif // FRAME_BUFFER_H