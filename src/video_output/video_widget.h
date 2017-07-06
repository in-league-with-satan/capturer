#ifndef VIDEO_WIDGET_H
#define VIDEO_WIDGET_H

#include <QWidget>

#include "frame_buffer.h"

class QAbstractVideoSurface;

class VideoSurface;
class VideoWidgetUpdateThread;

class VideoWidget: public QWidget
{
    Q_OBJECT

public:
    VideoWidget(QWidget *parent=0);
    ~VideoWidget();

    FrameBuffer::ptr frameBuffer();

    QSize sizeHint() const;

    void fillBlack();

protected:
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);

private:
    VideoSurface *surface;

    FrameBuffer::ptr frame_buffer;

    VideoWidgetUpdateThread *update_thread;

};

#endif // VIDEO_WIDGET_H
