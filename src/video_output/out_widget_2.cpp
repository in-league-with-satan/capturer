#include <QApplication>
#include <QDebug>
#include <QLayout>
#include <QVideoSurfaceFormat>
#include <QMutex>
#include <QResizeEvent>


#include "frame_buffer.h"

#include "video_surface.h"
#include "video_widget.h"
#include "video_widget_gl.h"

#include "out_widget_2.h"

OutWidget2::OutWidget2(QWidget *parent)
    : QWidget(parent)
{
    video_widget=(VideoWidgetGl*)new VideoWidget(this);

    setFocusPolicy(Qt::StrongFocus);

    //

    frame_buffer=new FrameBuffer(this);
    frame_buffer->setMaxSize(2);

    //

    thread=new OutWidgetUpdateThread(frame_buffer, video_widget->videoSurface(), this);
}

OutWidget2::~OutWidget2()
{
}

FrameBuffer *OutWidget2::frameBuffer()
{
    return frame_buffer;
}

void OutWidget2::resizeEvent(QResizeEvent *event)
{
    video_widget->resize(event->size());
}

//

OutWidgetUpdateThread::OutWidgetUpdateThread(FrameBuffer *frame_buffer, QAbstractVideoSurface *surface, QObject *parent)
    : QThread(parent)
    , frame_buffer(frame_buffer)
    , surface(surface)
{
    setTerminationEnabled(true);

    start();
}

void OutWidgetUpdateThread::run()
{
    Frame::ptr frame;

    while(true) {
        if(frame_buffer->isEmpty())
            frame_buffer->wait();

        frame=frame_buffer->take();

        if(frame) {
            if(!surface->isActive())
                surface->start(QVideoSurfaceFormat(frame->video.decklink_frame.getSize(), QVideoFrame::Format_ARGB32));

            ((VideoSurface*)surface)->present(frame);

            frame.reset();
        }
    }
}
