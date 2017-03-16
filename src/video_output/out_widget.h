#ifndef OUT_WIDGET_H
#define OUT_WIDGET_H

#include <QGLWidget>
#include <QImage>
#include <QTimer>

class FrameBuffer;

class OutWidget : public QGLWidget
{
    Q_OBJECT

public:
    explicit OutWidget(QWidget *parent=0);
    virtual ~OutWidget();

    FrameBuffer *frameBuffer();

protected:
    virtual void initializeGL();
    virtual void resizeGL(int width, int height);
    virtual void paintGL();

    virtual void paintEvent(QPaintEvent *event);

    virtual void leaveEvent(QEvent*);
    virtual void enterEvent(QEvent*);

private slots:
    void checkFrame();

private:
    QImage img_frame;

    FrameBuffer *frame_buffer;

    QTimer *timer;
};

#endif // OUT_WIDGET_H
