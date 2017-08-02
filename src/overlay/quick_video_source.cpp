#include <QDebug>
#include <QApplication>

#include <QtQml/qqml.h>

#include "quick_video_source.h"

QuickVideoSource::QuickVideoSource(QObject *parent)
    : QThread(parent)
    , surface(nullptr)
{
    frame_buffer=FrameBuffer::make();
    frame_buffer->setMaxSize(1);

    start(QThread::LowPriority);
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

void QuickVideoSource::run()
{
    Frame::ptr frame;

    running=true;

    while(running) {
        frame_buffer->wait();

        frame=frame_buffer->take();

        if(!frame)
            continue;

        if(!surface)
            continue;

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

void QuickVideoSource::closeSurface()
{
    if(surface && surface->isActive() )
        surface->stop();
}

