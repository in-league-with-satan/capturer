#include <QDebug>
#include <QWidget>
#include <QVideoSurfaceFormat>
#include <QPainter>

#include "video_surface.h"

VideoSurface::VideoSurface(QWidget *widget, QObject *parent)
    : QAbstractVideoSurface(parent)
    , widget(widget)
    , image_format(QImage::Format_Invalid)
{
}

QList<QVideoFrame::PixelFormat> VideoSurface::supportedPixelFormats(QAbstractVideoBuffer::HandleType handle_type) const
{
    if(handle_type==QAbstractVideoBuffer::NoHandle) {
        return QList<QVideoFrame::PixelFormat>()
                << QVideoFrame::Format_ARGB32
                << QVideoFrame::Format_BGRA32
                << QVideoFrame::Format_YUV444
                << QVideoFrame::Format_YUV420P;
    }

    return QList<QVideoFrame::PixelFormat>();
}

bool VideoSurface::isFormatSupported(const QVideoSurfaceFormat &format) const
{
    const QImage::Format image_format=QVideoFrame::imageFormatFromPixelFormat(format.pixelFormat());

    const QSize size=format.frameSize();

    return image_format!=QImage::Format_Invalid
            && !size.isEmpty()
            && format.handleType()==QAbstractVideoBuffer::NoHandle;
}

bool VideoSurface::start(const QVideoSurfaceFormat &format)
{
    const QImage::Format image_format=QVideoFrame::imageFormatFromPixelFormat(format.pixelFormat());

    const QSize size=format.frameSize();

    if(image_format!=QImage::Format_Invalid && !size.isEmpty()) {
        this->image_format=image_format;

        image_size=size;

        rect_source=format.viewport();

        if(!QAbstractVideoSurface::start(format)) {
            qCritical() << "QAbstractVideoSurface::start err:" << error();
        }

        widget->updateGeometry();

        updateVideoRect();

        return true;
    }

    return false;
}

void VideoSurface::stop()
{
    QMutexLocker ml(&mutex);

    frame.reset();

    rect_target=QRect();

    QAbstractVideoSurface::stop();

    widget->update();
}

bool VideoSurface::present(const QVideoFrame &frame)
{
    Q_UNUSED(frame)

    qWarning() << "VideoSurface::present(QVideoFrame) called";

    return false;
}

void VideoSurface::present(Frame::ptr frame)
{
    mutex.lock();

    this->frame=frame;

    QSize frame_size=frame->video.decklink_frame.getSize();

    if(rect_source.size()!=frame_size) {
        rect_source.setSize(frame_size);

        mutex.unlock();

        stop();
        start(QVideoSurfaceFormat(frame_size, QVideoFrame::Format_ARGB32));

        return;
    }

    mutex.unlock();
}

QRect VideoSurface::videoRect() const
{
    return rect_target;
}

void VideoSurface::updateVideoRect()
{
    QSize size=surfaceFormat().sizeHint();

    size.scale(widget->size(), Qt::KeepAspectRatio);


    rect_target=QRect(QPoint(0, 0), size);

    rect_target.moveCenter(widget->rect().center());
}

void VideoSurface::paint(QPainter *painter, bool smooth_transform)
{
    {
        QMutexLocker ml(&mutex);

        if(!frame)
            return;
    }

    if(smooth_transform) {
        painter->save();

        painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
    }

    Frame::ptr frame_tmp;

    {
        QMutexLocker ml(&mutex);

        frame_tmp=frame;
    }

    const QTransform old_transform=painter->transform();

    if(surfaceFormat().scanLineDirection()==QVideoSurfaceFormat::BottomToTop) {
        painter->scale(1, -1);

        painter->translate(0, -widget->height());
    }

    painter->drawImage(rect_target,
                       QImage((uchar*)frame_tmp->video.raw->data(),
                              frame_tmp->video.decklink_frame.getSize().width(),
                              frame_tmp->video.decklink_frame.getSize().height(),
                              image_format),
                       rect_source);

    painter->setTransform(old_transform);

    if(smooth_transform)
        painter->restore();
}
