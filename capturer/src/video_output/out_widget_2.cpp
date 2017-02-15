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
    frame_buffer->setMaxBufferSize(1);
    frame_buffer->setDropSkipped(true);

    timer=new QTimer(this);
    timer->setInterval(2);

    connect(timer, SIGNAL(timeout()), SLOT(checkFrame()));

    timer->start();
}

OutWidget2::~OutWidget2()
{
}

FrameBuffer *OutWidget2::frameBuffer()
{
    return frame_buffer;
}

void OutWidget2::focusInEvent(QFocusEvent *)
{
    emit focusEvent();
}

void OutWidget2::checkFrame()
{
    FrameBuffer::Frame frame;

    timer->stop();

    {
        QMutexLocker ml(frame_buffer->mutex_frame_buffer);

        if(frame_buffer->queue.isEmpty()) {
            timer->start();
            return;
        }

        frame=frame_buffer->queue.dequeue();
    }

    QVideoFrame video_frame=QVideoFrame(QImage((uchar*)frame.ba_video.data(), frame.size_video.width(), frame.size_video.height(), QImage::Format_ARGB32));

    if(!video_widget->videoSurface()->isActive()) {
        video_widget->videoSurface()->start(QVideoSurfaceFormat(video_frame.size(), video_frame.pixelFormat()));

    }

    video_widget->videoSurface()->present(video_frame);

    timer->start();
}
