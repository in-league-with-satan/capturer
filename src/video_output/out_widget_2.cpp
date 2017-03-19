#include <QApplication>
#include <QDebug>
#include <QLayout>
#include <QVideoSurfaceFormat>
#include <QMutex>


#include "frame_buffer.h"

#include "video_surface.h"
#include "video_widget.h"
#include "video_widget_gl.h"

#include "out_widget_2.h"

OutWidget2::OutWidget2(QWidget *parent)
    : QWidget(parent)
{
    video_widget=(VideoWidgetGl*)new VideoWidget();

    setFocusPolicy(Qt::StrongFocus);

    QVBoxLayout *la_main=new QVBoxLayout();

    la_main->setMargin(0);

    la_main->addWidget(video_widget);

    setLayout(la_main);

    //

    frame_buffer=new FrameBuffer(QMutex::Recursive, this);
    frame_buffer->setMaxBufferSize(2);

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

void OutWidget2::focusInEvent(QFocusEvent*)
{
    emit focusEvent();
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
    QVideoFrame video_frame;

    bool queue_is_empty;

    while(true) {
wait:

        frame_buffer->event.wait();

nowait:

        {
            QMutexLocker ml(frame_buffer->mutex_frame_buffer);

            if(frame_buffer->queue.isEmpty()) {
                goto wait;
            }

            frame=frame_buffer->queue.dequeue();

            queue_is_empty=frame_buffer->queue.isEmpty();
        }

        video_frame=QVideoFrame(QImage((uchar*)frame->video.raw.data(), frame->video.size.width(), frame->video.size.height(), QImage::Format_ARGB32));

        frame.reset();

        if(!surface->isActive())
            surface->start(QVideoSurfaceFormat(video_frame.size(), video_frame.pixelFormat()));

        surface->present(video_frame);

        if(!queue_is_empty)
            goto nowait;
    }
}
