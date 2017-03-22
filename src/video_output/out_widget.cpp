#include <QApplication>
#include <QDebug>
#include <QMutex>

#include "frame_buffer.h"

#include "out_widget.h"

OutWidget::OutWidget(QWidget *parent) :
    QGLWidget(parent)
{
    frame_buffer=new FrameBuffer(this);
    frame_buffer->setMaxSize(1);

    timer=new QTimer(this);
    timer->setInterval(10);

    connect(timer, SIGNAL(timeout()), SLOT(checkFrame()));

    timer->start();
}

OutWidget::~OutWidget()
{
}

FrameBuffer *OutWidget::frameBuffer()
{
    return frame_buffer;
}

void OutWidget::initializeGL()
{
    qglClearColor(Qt::black);

    glEnable(GL_DEPTH_TEST);
}

void OutWidget::resizeGL(int width, int height)
{
    glViewport(0, 0, (GLint)width, (GLint)height);
}

void OutWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OutWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter p(this);

    if(img_frame.isNull())
        p.fillRect(QRect(QPoint(0, 0), size()), Qt::black);

    else
        p.drawImage(QPoint(0, 0), img_frame);

    p.end();
}

void OutWidget::leaveEvent(QEvent*)
{
    QApplication::setOverrideCursor(Qt::ArrowCursor);
}

void OutWidget::enterEvent(QEvent*)
{
    QApplication::setOverrideCursor(Qt::BlankCursor);
}

void OutWidget::checkFrame()
{
    Frame::ptr frame;

    timer->stop();


    if(frame_buffer->isEmpty()) {
        timer->start();
        return;
    }

    frame=frame_buffer->take();


//    timer->start();
//    return;

    QImage img_tmp=QImage((uchar*)frame->video.raw.data(), frame->video.size.width(), frame->video.size.height(), QImage::Format_ARGB32);

    frame.reset();

    if(img_tmp.size()!=size()) {
        if(img_tmp.size().width()>1920)
            img_frame=img_tmp.scaled(size(), Qt::IgnoreAspectRatio, Qt::FastTransformation);

        else
            img_frame=img_tmp.scaled(size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    } else
        img_frame=img_tmp.copy();

    update();

    timer->start();
}
