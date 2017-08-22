#include <QDebug>
#include <QApplication>

#include <QtQml/qqml.h>

#include "quick_video_source.h"


QuickVideoSource::QuickVideoSource(QObject *parent)
    : QThread(parent)
    , surface(nullptr)
    , half_fps(false)
{
    frame_buffer=FrameBuffer::make();
    frame_buffer->setMaxSize(1);

    startTimer(1);
}

QuickVideoSource::QuickVideoSource(bool thread, QObject *parent)
    : QThread(parent)
    , surface(nullptr)
    , half_fps(false)
{
    frame_buffer=FrameBuffer::make();
    frame_buffer->setMaxSize(1);

    if(thread) {
        start(QThread::LowPriority);
        // start(QThread::NormalPriority);

    } else
        startTimer(1);
}

QuickVideoSource::~QuickVideoSource()
{
    closeSurface();

    running=false;

    frame_buffer->append(nullptr);

    while(isRunning()) {
        msleep(30);
    }
}

FrameBuffer::ptr QuickVideoSource::frameBuffer()
{
    return frame_buffer;
}

QAbstractVideoSurface *QuickVideoSource::videoSurface() const
{
    return surface;
}

void QuickVideoSource::setVideoSurface(QAbstractVideoSurface *s)
{
    closeSurface();

    surface=s;
}

void QuickVideoSource::setHalfFps(bool value)
{
    half_fps=value;
}

void QuickVideoSource::run()
{
    Frame::ptr frame;

    bool skip_frame=false;

    running=true;

    while(running) {
        frame_buffer->wait();

        frame=frame_buffer->take();

        if(!frame)
            continue;

        if(!surface)
            continue;

        skip_frame=!skip_frame;

        if(half_fps) {
            if(skip_frame) {
                frame.reset();
                continue;
            }
        }

        if(frame->video.decklink_frame.getSize()!=format.frameSize() || QVideoFrame::Format_ARGB32!=format.pixelFormat()) {
            closeSurface();

            format=
                    QVideoSurfaceFormat(frame->video.decklink_frame.getSize(),
                                        QVideoFrame::Format_ARGB32);

            if(!surface->start(format)) {
                qCritical() << "surface->start error" << surface->error();
                format=QVideoSurfaceFormat();
                continue;
            }
        }

        last_image=QImage((uchar*)frame->video.raw->constData(),
                          frame->video.decklink_frame.getSize().width(),
                          frame->video.decklink_frame.getSize().height(),
                          frame->video.decklink_frame.GetRowBytes(),
                          QImage::Format_ARGB32);


        last_frame=QVideoFrame(last_image);

        last_frame.map(QAbstractVideoBuffer::ReadOnly);

        surface->present(last_frame);

        last_frame.unmap();

        frame.reset();
    }
}

void QuickVideoSource::timerEvent(QTimerEvent*)
{
    if(!surface)
        return;

    Frame::ptr frame=
            frame_buffer->take();

    if(!frame)
        return;

    if(frame->video.decklink_frame.getSize()!=format.frameSize() || QVideoFrame::Format_ARGB32!=format.pixelFormat()) {
        closeSurface();

        format=
                QVideoSurfaceFormat(frame->video.decklink_frame.getSize(),
                                    QVideoFrame::Format_ARGB32);

        if(!surface->start(format)) {
            qCritical() << "surface->start error" << surface->error();
            format=QVideoSurfaceFormat();
            return;
        }
    }

    last_image=QImage((uchar*)frame->video.raw->constData(),
                      frame->video.decklink_frame.getSize().width(),
                      frame->video.decklink_frame.getSize().height(),
                      frame->video.decklink_frame.GetRowBytes(),
                      QImage::Format_ARGB32);

    last_frame=QVideoFrame(last_image);

    last_frame.map(QAbstractVideoBuffer::ReadOnly);

    surface->present(last_frame);

    last_frame.unmap();
}

void QuickVideoSource::closeSurface()
{
    if(surface && surface->isActive() )
        surface->stop();
}
