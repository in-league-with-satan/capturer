#ifndef VIDEO_WIDGET_SURFACE_H
#define VIDEO_WIDGET_SURFACE_H

#include <QAbstractVideoSurface>
#include <QImage>
#include <QRect>
#include <QVideoFrame>

class VideoWidgetSurface : public QAbstractVideoSurface
{
    Q_OBJECT

public:
    VideoWidgetSurface(QWidget *widget, QObject *parent=0);

    QList<QVideoFrame::PixelFormat> supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType=QAbstractVideoBuffer::NoHandle) const;

    bool isFormatSupported(const QVideoSurfaceFormat &format) const;

    bool start(const QVideoSurfaceFormat &format);
    void stop();

    bool present(const QVideoFrame &frame);

    QRect videoRect() const;

    void updateVideoRect();

    void paint(QPainter *painter);

private:
    QWidget *widget;
    QImage::Format image_format;
    QRect rect_target;
    QRect rect_source;
    QSize image_size;
    QVideoFrame current_frame;
};

#endif // VIDEO_WIDGET_SURFACE_H
