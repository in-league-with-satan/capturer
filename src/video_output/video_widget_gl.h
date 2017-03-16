#ifndef VIDEO_WIDGET_GL_H
#define VIDEO_WIDGET_GL_H

#include <QGLWidget>

class QAbstractVideoSurface;

class VideoSurface;

class VideoWidgetGl: public QGLWidget
{
    Q_OBJECT

public:
    VideoWidgetGl(QWidget *parent=0);
    ~VideoWidgetGl();

    QAbstractVideoSurface *videoSurface() const;

    QSize sizeHint() const;

protected:
    virtual void initializeGL();
    virtual void resizeGL(int width, int height);
    virtual void paintGL();

    virtual void paintEvent(QPaintEvent *event);
    virtual void resizeEvent(QResizeEvent *event);

private:
    VideoSurface *surface;
};

#endif // VIDEO_WIDGET_GL_H
