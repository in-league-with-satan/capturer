#ifndef QUICK_VIDEO_SOURCE_H
#define QUICK_VIDEO_SOURCE_H

#include <QThread>
#include <QAbstractVideoSurface>
#include <QVideoSurfaceFormat>

#include <atomic>

#include "frame_buffer.h"

class AVFrame;

class FFFormatConverter;

class DecklinkFrameConverter;

class QuickVideoSource : public QThread
{
    Q_OBJECT

    Q_PROPERTY(QAbstractVideoSurface *videoSurface READ videoSurface WRITE setVideoSurface)

public:
    explicit QuickVideoSource(QObject *parent=0);
    explicit QuickVideoSource(bool thread, QObject *parent=0);
    ~QuickVideoSource();

    FrameBuffer::ptr frameBuffer();

    QAbstractVideoSurface *videoSurface() const;
    void setVideoSurface(QAbstractVideoSurface *s);

    void setHalfFps(bool value);

    bool fastYuv() const;
    void setFastYuv(bool value);

protected:
    virtual void run();
    virtual void timerEvent(QTimerEvent*);

private:
    void closeSurface();

private:
    QAbstractVideoSurface *surface;
    QVideoSurfaceFormat format;

    FrameBuffer::ptr frame_buffer;

    QImage last_image;
    QVideoFrame last_frame;

    std::atomic <bool> running;
    std::atomic <bool> half_fps;
    std::atomic <bool> fast_yuv;

    AVFrame *yuv_src;
    AVFrame *yuv_dst;

    FFFormatConverter *format_converter_ff;
    DecklinkFrameConverter *format_converter_dl;
};

#endif // QUICK_VIDEO_SOURCE_H
