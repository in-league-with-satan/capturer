#ifndef OUT_WIDGET_H
#define OUT_WIDGET_H

#include <QGLWidget>
#include <QImage>

class OutWidget : public QGLWidget
{
    Q_OBJECT

public:
    explicit OutWidget(QWidget *parent=0);
    virtual ~OutWidget();

public slots:
    void frame(QImage image);

protected:
    virtual void initializeGL();
    virtual void resizeGL(int width, int height);
    virtual void paintGL();

    virtual void paintEvent(QPaintEvent *event);

private:
    QImage img_frame;

};

#endif // OUT_WIDGET_H
