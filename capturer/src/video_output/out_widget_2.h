#ifndef OUT_WIDGET_2_H
#define OUT_WIDGET_2_H

#include <QWidget>
#include <QImage>
#include <QTimer>

class FrameBuffer;
class VideoWidget;

class OutWidget2 : public QWidget
{
    Q_OBJECT

public:
    explicit OutWidget2(QWidget *parent=0);
    virtual ~OutWidget2();

    FrameBuffer *frameBuffer();

protected:
    virtual void leaveEvent(QEvent*);
    virtual void enterEvent(QEvent*);

private slots:
    void checkFrame();

private:
    QImage img_frame;

    FrameBuffer *frame_buffer;

    VideoWidget *video_widget;

    QTimer *timer;
};

#endif // OUT_WIDGET_2_H
