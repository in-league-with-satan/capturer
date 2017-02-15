#ifndef OUT_WIDGET_2_H
#define OUT_WIDGET_2_H

#include <QWidget>
#include <QImage>
#include <QTimer>

class FrameBuffer;
class VideoWidgetGl;

class OutWidget2 : public QWidget
{
    Q_OBJECT

public:
    explicit OutWidget2(QWidget *parent=0);
    virtual ~OutWidget2();

    FrameBuffer *frameBuffer();

protected:
    virtual void focusInEvent(QFocusEvent *);

private slots:
    void checkFrame();

private:
    QImage img_frame;

    FrameBuffer *frame_buffer;

    VideoWidgetGl *video_widget;

    QTimer *timer;

signals:
    void focusEvent();
};

#endif // OUT_WIDGET_2_H
