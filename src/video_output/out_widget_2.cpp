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

    while(true) {
        if(frame_buffer->isEmpty())
            frame_buffer->wait();

        frame=frame_buffer->take();

        if(frame) {
            if(!surface->isActive())
                surface->start(QVideoSurfaceFormat(frame->video.size, QVideoFrame::Format_ARGB32));

            ((VideoSurface*)surface)->present(frame);

            frame.reset();
        }
    }
}
