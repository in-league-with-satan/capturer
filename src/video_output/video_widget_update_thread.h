#ifndef VIDEO_WIDGET_UPDATE_THREAD_H
#define VIDEO_WIDGET_UPDATE_THREAD_H

#include <QWidget>
#include <QThread>

#include <atomic>

#include "frame_buffer.h"


class VideoSurface;
class VideoWidget;
class VideoWidgetUpdateThread;

class VideoWidgetUpdateThread : public QThread
{
    Q_OBJECT

public:
    VideoWidgetUpdateThread(FrameBuffer::ptr frame_buffer, VideoSurface *surface, QWidget *widget, QObject *parent=0);
    ~VideoWidgetUpdateThread();

protected:
    void run();

private:
    FrameBuffer::ptr frame_buffer;
    VideoSurface *surface;
    QWidget *widget;

    std::atomic <bool> running;

signals:
    void update();
};

#endif // VIDEO_WIDGET_UPDATE_THREAD_H
