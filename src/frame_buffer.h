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
    typedef std::shared_ptr<FrameBuffer> ptr;

    explicit FrameBuffer(QObject *parent=0);
    ~FrameBuffer();

    static ptr make() {
        return ptr(new FrameBuffer());
    }

    void append(Frame::ptr frame);

    Frame::ptr take();

    void wait();

    void setMaxSize(uint16_t size);

    void setEnabled(bool value);
    bool isEnabled();

    void clear();

    bool isEmpty();

    QPair <int, int> size();

private:
    QQueue <Frame::ptr> queue;
    QMutex mutex;
    uint16_t max_size;
    EventWaiting event;
    bool enabled;

signals:
    void frameSkipped();
};

#endif // FRAME_BUFFER_H
