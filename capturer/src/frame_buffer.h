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
        Frame() {
            reset_counter=false;
        }

        QByteArray ba_video;
        QSize size_video;
        QByteArray ba_audio;
        uint32_t bmd_pixel_format;
        uint8_t counter;
        bool reset_counter;
    };

    void appendFrame(Frame frame);

    void setMaxBufferSize(uint16_t size);

    void setDropSkipped(bool state);

    void setEnabled(bool value);

    void clear();

    QQueue <Frame> queue;

    QMutex *mutex_frame_buffer;

private:
    uint16_t buffer_max_size;

    bool drop_skipped;
    bool enabled;

signals:
    void frameSkipped();
};

#endif // FRAME_BUFFER_H
