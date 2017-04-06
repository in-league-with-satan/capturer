#ifndef OUT_WIDGET_2_H
#define OUT_WIDGET_2_H

#include <QWidget>
#include <QImage>
#include <QThread>

class QAbstractVideoSurface;

class FrameBuffer;
class VideoWidgetGl;
class VideoWidget;
class OutWidgetUpdateThread;

class OutWidget2 : public QWidget
{
    Q_OBJECT

public:
    explicit OutWidget2(QWidget *parent=0);
    virtual ~OutWidget2();

    FrameBuffer *frameBuffer();

protected:
    virtual void resizeEvent(QResizeEvent *event);

private:
    FrameBuffer *frame_buffer;
    VideoWidget *video_widget;
    OutWidgetUpdateThread *thread;
};

//

class OutWidgetUpdateThread : public QThread
{
    Q_OBJECT

public:
    OutWidgetUpdateThread(FrameBuffer *frame_buffer, QAbstractVideoSurface *surface, QObject *parent=0);

protected:
    void run();

private:
    FrameBuffer *frame_buffer;
    QAbstractVideoSurface *surface;

signals:
    void update();
};

#endif // OUT_WIDGET_2_H
