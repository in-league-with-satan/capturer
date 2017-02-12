#include <QVideoSurfaceFormat>
#include <QPainter>
#include <QPaintEvent>

#include "video_surface.h"


#include "video_widget_gl.h"

VideoWidgetGl::VideoWidgetGl(QWidget *parent)
    : QGLWidget(parent)
    , surface(0)
{
    setAutoFillBackground(false);

    setAttribute(Qt::WA_NoSystemBackground, true);


    QPalette palette=this->palette();

    palette.setColor(QPalette::Background, Qt::black);

    setPalette(palette);


    surface=new VideoSurface(this);
}

VideoWidgetGl::~VideoWidgetGl()
{
    delete surface;
}

QAbstractVideoSurface *VideoWidgetGl::videoSurface() const
{
    return surface;
}

QSize VideoWidgetGl::sizeHint() const
{
    return surface->surfaceFormat().sizeHint();
}

void VideoWidgetGl::initializeGL()
{
    qglClearColor(Qt::black);

    glEnable(GL_DEPTH_TEST);
}

void VideoWidgetGl::resizeGL(int width, int height)
{
    glViewport(0, 0, (GLint)width, (GLint)height);
}

void VideoWidgetGl::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void VideoWidgetGl::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    if(surface->isActive()) {
        const QRect video_rect=surface->videoRect();

        if(!video_rect.contains(event->rect())) {
            QRegion region=event->region();

            region=region.subtracted(video_rect);


            QBrush brush=palette().background();

            foreach(const QRect &rect, region.rects())
                painter.fillRect(rect, brush);
        }

        surface->paint(&painter);

    } else {
        painter.fillRect(event->rect(), palette().background());
    }
}

void VideoWidgetGl::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    surface->updateVideoRect();
}

