#include <QWidget>
#include <QVideoSurfaceFormat>
#include <QPainter>
#include <QMutexLocker>

#include "video_surface.h"

VideoSurface::VideoSurface(QWidget *widget, QObject *parent)
    : QAbstractVideoSurface(parent)
    , widget(widget)
    , image_format(QImage::Format_Invalid)
{
}

QList<QVideoFrame::PixelFormat> VideoSurface::supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType) const
{
    if(handleType==QAbstractVideoBuffer::NoHandle) {
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
    QMutexLocker ml(&mutex);

    const QImage::Format image_format=QVideoFrame::imageFormatFromPixelFormat(format.pixelFormat());

    const QSize size=format.frameSize();

    if(image_format!=QImage::Format_Invalid && !size.isEmpty()) {
        this->image_format=image_format;

        image_size=size;

        rect_source=format.viewport();

        QAbstractVideoSurface::start(format);

        widget->updateGeometry();

        updateVideoRect();

        return true;
    }

    return false;
}

void VideoSurface::stop()
{
    current_frame=QVideoFrame();

    rect_target=QRect();

    QAbstractVideoSurface::stop();

    widget->update();
}

bool VideoSurface::present(const QVideoFrame &frame)
{
    QMutexLocker ml(&mutex);

    if(surfaceFormat().pixelFormat()!=frame.pixelFormat()
            || surfaceFormat().frameSize()!=frame.size()) {
        setError(IncorrectFormatError);

        stop();

        return false;

    }

    current_frame=frame;

    // if(!widget->testAttribute(Qt::WA_WState_InPaintEvent))
    //     widget->repaint(rect_target);

    widget->update();

    return true;
}

QRect VideoSurface::videoRect() const
{
    return rect_target;
}

void VideoSurface::updateVideoRect()
{
    QSize size=surfaceFormat().sizeHint();

    size.scale(widget->size().boundedTo(size), Qt::KeepAspectRatio);


    rect_target=QRect(QPoint(0, 0), size);

    rect_target.moveCenter(widget->rect().center());
}

void VideoSurface::paint(QPainter *painter)
{
    // painter->setRenderHint(QPainter::SmoothPixmapTransform, true);

    if(current_frame.map(QAbstractVideoBuffer::ReadOnly)) {
        const QTransform old_transform=painter->transform();

        if(surfaceFormat().scanLineDirection()==QVideoSurfaceFormat::BottomToTop) {
           painter->scale(1, -1);

           painter->translate(0, -widget->height());
        }

        QImage image(current_frame.bits(),
                     current_frame.width(),
                     current_frame.height(),
                     current_frame.bytesPerLine(),
                     image_format);

        painter->drawImage(rect_target, image, rect_source);

        painter->setTransform(old_transform);

    }

    if(current_frame.isMapped())
        current_frame.unmap();
}
