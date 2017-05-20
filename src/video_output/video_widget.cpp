#include <QVideoSurfaceFormat>
#include <QPainter>
#include <QPaintEvent>

#include "video_surface.h"
#include "frame_buffer.h"
#include "video_widget_update_thread.h"

#include "video_widget.h"

VideoWidget::VideoWidget(QWidget *parent)
    : QWidget(parent)
    , surface(0)
{
    setAutoFillBackground(false);

    frame_buffer=new FrameBuffer(this);
    frame_buffer->setMaxSize(2);

    QPalette palette=this->palette();

    palette.setColor(QPalette::Background, Qt::black);

    setPalette(palette);


    surface=new VideoSurface(this);

    update_thread=new VideoWidgetUpdateThread(frame_buffer, surface, this, this);

    connect(update_thread, SIGNAL(update()), SLOT(repaint()), Qt::QueuedConnection);
}

VideoWidget::~VideoWidget()
{
}

FrameBuffer *VideoWidget::frameBuffer()
{
    return frame_buffer;
}

QSize VideoWidget::sizeHint() const
{
    return surface->surfaceFormat().sizeHint();
}

void VideoWidget::fillBlack()
{
    QImage image(640, 480, QImage::Format_ARGB32);

    image.fill(Qt::black);

    Frame::ptr frame=Frame::make();

    frame->video.decklink_frame.init(image.size(), bmdFormat8BitARGB);

    memcpy(frame->video.raw->data(), image.bits(), frame->video.raw->size());

    frame_buffer->append(frame);
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

        surface->paint(&painter, false);

    } else {
        painter.fillRect(event->rect(), palette().background());
    }
}

void VideoWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    surface->updateVideoRect();
}

