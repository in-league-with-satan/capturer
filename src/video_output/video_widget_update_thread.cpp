#include <QApplication>
#include <QDebug>
#include <QLayout>
#include <QVideoSurfaceFormat>

#include "frame_buffer.h"

#include "video_surface.h"
#include "video_widget.h"

#include "video_widget_update_thread.h"

VideoWidgetUpdateThread::VideoWidgetUpdateThread(FrameBuffer *frame_buffer, VideoSurface *surface, QWidget *widget, QObject *parent)
    : QThread(parent)
    , frame_buffer(frame_buffer)
    , surface(surface)
    , widget(widget)
{
    setTerminationEnabled(true);

    start();
}

void VideoWidgetUpdateThread::run()
{
    Frame::ptr frame;

    while(true) {
        if(frame_buffer->isEmpty())
            frame_buffer->wait();

        frame=frame_buffer->take();

        if(frame) {
            if(!surface->isActive())
                surface->start(QVideoSurfaceFormat(frame->video.decklink_frame.getSize(), QVideoFrame::Format_ARGB32));

            surface->present(frame);

            widget->update();

            frame.reset();
        }
    }
}