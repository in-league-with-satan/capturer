#ifndef FRAME_BUFFER_H
#define FRAME_BUFFER_H

#include <QObject>
#include <QMutex>
#include <QQueue>
#include <QSize>

#include "event_waiting.h"
#include "frame.h"

class FrameBuffer : public QObject
{
    Q_OBJECT

public:
    explicit FrameBuffer(QMutex::RecursionMode recursion_mode=QMutex::Recursive, QObject *parent=0);
    ~FrameBuffer();

    void appendFrame(Frame::ptr frame);

    void setMaxBufferSize(uint16_t size);

    void setEnabled(bool value);

    void clear();

    QPair <int, int> size();

    QQueue <Frame::ptr> queue;

    QMutex *mutex_frame_buffer;

    EventWaiting event;

private:
    uint16_t buffer_max_size;

    bool enabled;

signals:
    void frameSkipped();
};

#endif // FRAME_BUFFER_H
