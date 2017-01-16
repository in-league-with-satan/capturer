#include <QApplication>
#include <QDebug>
#include <QMutex>

#include "frame_buffer.h"

#include "out_widget.h"

OutWidget::OutWidget(QWidget *parent) :
    QGLWidget(parent)
{
    frame_buffer=new FrameBuffer(QMutex::Recursive, this);
    frame_buffer->setMaxBufferSize(1);
    frame_buffer->setDropSkipped(true);

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

//    timer->start();
//    return;

    QImage img_tmp=QImage((uchar*)frame.ba_video.data(), frame.size_video.width(), frame.size_video.height(), QImage::Format_ARGB32);

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
