#ifndef VIDEO_WIDGET_H
#define VIDEO_WIDGET_H

#include <QWidget>

class QAbstractVideoSurface;

class VideoSurface;
class FrameBuffer;
class VideoWidgetUpdateThread;

class VideoWidget: public QWidget
{
    Q_OBJECT

public:
    VideoWidget(QWidget *parent=0);
    ~VideoWidget();

    FrameBuffer *frameBuffer();

    QSize sizeHint() const;

protected:
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);

private:
    VideoSurface *surface;

    FrameBuffer *frame_buffer;

    VideoWidgetUpdateThread *update_thread;
};

#endif // VIDEO_WIDGET_H
