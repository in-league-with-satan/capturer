#include <QDebug>

#include "out_widget.h"

OutWidget::OutWidget(QWidget *parent) :
    QGLWidget(parent)
{
}

OutWidget::~OutWidget()
{
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

void OutWidget::frame(QImage image)
{
    if(image.isNull()) {
        qCritical() << "OutWidget::frame: null image";
        return;
    }

    if(image.size()!=size())
        img_frame=image.scaled(size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    else
        img_frame=image.copy();

    update();
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
