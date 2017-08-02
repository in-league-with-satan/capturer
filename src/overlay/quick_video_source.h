#ifndef QUICK_VIDEO_SOURCE_H
#define QUICK_VIDEO_SOURCE_H

#include <QThread>
#include <QAbstractVideoSurface>
#include <QVideoSurfaceFormat>

#include <atomic>

#include "frame_buffer.h"

class QuickVideoSource : public QThread
{
    Q_OBJECT

    Q_PROPERTY(QAbstractVideoSurface *videoSurface READ videoSurface WRITE setVideoSurface)

public:
    explicit QuickVideoSource(QObject *parent=0);
    ~QuickVideoSource();

    FrameBuffer::ptr frameBuffer();

    QAbstractVideoSurface *videoSurface() const;
    void setVideoSurface(QAbstractVideoSurface *s);

protected:
    void run();

private:
    void closeSurface();

private:
    QAbstractVideoSurface *surface;
    QVideoSurfaceFormat format;

    FrameBuffer::ptr frame_buffer;

    QImage last_image;
    QVideoFrame last_frame;

    std::atomic <bool> running;
};

#endif // QUICK_VIDEO_SOURCE_H
