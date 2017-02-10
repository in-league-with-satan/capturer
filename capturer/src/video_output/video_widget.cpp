#include <QVideoSurfaceFormat>
#include <QPainter>
#include <QPaintEvent>

#include "video_widget_surface.h"


#include "video_widget.h"

VideoWidget::VideoWidget(QWidget *parent)
    : QWidget(parent)
    , surface(0)
{
    setAutoFillBackground(false);

    setAttribute(Qt::WA_NoSystemBackground, true);


    QPalette palette=this->palette();

    palette.setColor(QPalette::Background, Qt::black);

    setPalette(palette);


    surface=new VideoWidgetSurface(this);
}

VideoWidget::~VideoWidget()
{
    delete surface;
}

QAbstractVideoSurface *VideoWidget::videoSurface() const
{
    return surface;
}

QSize VideoWidget::sizeHint() const
{
    return surface->surfaceFormat().sizeHint();
}

void VideoWidget::paintEvent(QPaintEvent *event)
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

void VideoWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    surface->updateVideoRect();
}

